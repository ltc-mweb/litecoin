#include <mweb/mweb_transact.h>
#include <key_io.h>

using namespace MWEB;

TxType MWEB::GetTxType(const std::vector<CRecipient>& recipients, const std::vector<CInputCoin>& input_coins)
{
    assert(!recipients.empty());

    static auto is_ltc = [](const CInputCoin& input) { return !input.IsMWEB(); };
    static auto is_mweb = [](const CInputCoin& input) { return input.IsMWEB(); };

    if (recipients.front().IsMWEB()) {
        // If any inputs are non-MWEB inputs, this is a peg-in transaction.
        // Otherwise, it's a simple MWEB-to-MWEB transaction.
        if (std::any_of(input_coins.cbegin(), input_coins.cend(), is_ltc)) {
            return TxType::PEGIN;
        } else {
            return TxType::MWEB_TO_MWEB;
        }
    } else {
        // If any inputs are MWEB inputs, this is a peg-out transaction.
        // NOTE: This does not exclude the possibility that it's also pegging-in in addition to the pegout.
        // Otherwise, if there are no MWEB inputs, it's a simple LTC-to-LTC transaction.
        if (std::any_of(input_coins.cbegin(), input_coins.cend(), is_mweb)) {
            return TxType::PEGOUT;
        } else {
            return TxType::LTC_TO_LTC;
        }
    }
}

bool Transact::CreateTx(
    const std::shared_ptr<MWEB::Wallet>& mweb_wallet,
    CMutableTransaction& transaction,
    const std::vector<CInputCoin>& selected_coins,
    const std::vector<CRecipient>& recipients,
    const CAmount& ltc_fee,
    const CAmount& mweb_fee,
    const bool include_mweb_change)
{
    TxType type = MWEB::GetTxType(recipients, selected_coins);
    if (type == TxType::LTC_TO_LTC) {
        return true;
    }

    // Add recipients
    std::vector<libmw::Recipient> mweb_recipients;
    for (const CRecipient& recipient : recipients) {
        if (recipient.IsMWEB()) {
            libmw::MWEBRecipient mweb_recipient{
                (uint64_t)recipient.nAmount,
                recipient.GetMWEBAddress()};

            mweb_recipients.push_back(std::move(mweb_recipient));
        } else {
            libmw::PegOutRecipient pegout_recipient{
                (uint64_t)recipient.nAmount,
                std::vector<uint8_t>(recipient.receiver.GetScript().begin(), recipient.receiver.GetScript().end())
            };
            mweb_recipients.push_back(std::move(pegout_recipient));
        }
    }

    // Add Change
    if (include_mweb_change) {
        mweb_recipients.push_back(BuildChangeRecipient(
            mweb_wallet,
            transaction,
            selected_coins,
            recipients,
            ltc_fee,
            mweb_fee));
    }

    // Calculate pegin_amount
    boost::optional<uint64_t> pegin_amount = boost::none;
    CAmount ltc_input_amount = GetLTCInputAmount(selected_coins);
    if (ltc_input_amount > 0) {
        assert(ltc_fee < ltc_input_amount);
        pegin_amount = (uint64_t)(ltc_input_amount - ltc_fee); // MW: TODO - There could also be LTC change
    }

    // Create transaction
    std::vector<libmw::Coin> input_coins = GetInputCoins(selected_coins);
    transaction.m_mwtx = libmw::wallet::CreateTx(
        input_coins,
        mweb_recipients,
        pegin_amount,
        (uint64_t)mweb_fee);

    // Update pegin output
    auto pegins = transaction.m_mwtx.GetPegIns();
    if (!pegins.empty()) {
        UpdatePegInOutput(transaction, pegins.front());
    }

    return true;
}

std::vector<libmw::Coin> Transact::GetInputCoins(const std::vector<CInputCoin>& inputs)
{
    std::vector<libmw::Coin> input_coins;
    for (const auto& coin : inputs) {
        if (coin.IsMWEB()) {
            input_coins.push_back(coin.GetMWEBCoin());
        }
    }

    return input_coins;
}

CAmount Transact::GetMWEBInputAmount(const std::vector<CInputCoin>& inputs)
{
    return std::accumulate(
        inputs.cbegin(), inputs.cend(), CAmount(0),
        [](CAmount amt, const CInputCoin& input) { return amt + (input.IsMWEB() ? input.GetAmount() : 0); });
}

CAmount Transact::GetLTCInputAmount(const std::vector<CInputCoin>& inputs)
{
    return std::accumulate(
        inputs.cbegin(), inputs.cend(), CAmount(0),
        [](CAmount amt, const CInputCoin& input) { return amt + (input.IsMWEB() ? 0 : input.GetAmount()); });
}

CAmount Transact::GetMWEBRecipientAmount(const std::vector<CRecipient>& recipients)
{
    return std::accumulate(
        recipients.cbegin(), recipients.cend(), CAmount(0),
        [](CAmount amt, const CRecipient& recipient) { return amt + (recipient.IsMWEB() ? recipient.nAmount : 0); });
}

bool Transact::UpdatePegInOutput(CMutableTransaction& transaction, const libmw::PegIn& pegin)
{
    for (size_t i = 0; i < transaction.vout.size(); i++) {
        if (IsPegInOutput(CTransaction(transaction).GetOutput(i))) {
            CScript pegin_script;
            pegin_script << CScript::EncodeOP_N(Consensus::Mimblewimble::WITNESS_VERSION);
            pegin_script << std::vector<uint8_t>(pegin.commitment.cbegin(), pegin.commitment.cend());
            transaction.vout[i].nValue = pegin.amount;
            transaction.vout[i].scriptPubKey = pegin_script;
            return true;
        }
    }

    return false;
}

libmw::Recipient Transact::BuildChangeRecipient(
    const std::shared_ptr<MWEB::Wallet>& mweb_wallet,
    CMutableTransaction& transaction,
    const std::vector<CInputCoin>& selected_coins,
    const std::vector<CRecipient>& recipients,
    const CAmount& ltc_fee,
    const CAmount& mweb_fee)
{
    boost::optional<uint64_t> pegin_amount = boost::none;
    CAmount ltc_input_amount = GetLTCInputAmount(selected_coins);
    if (ltc_input_amount > 0) {
        assert(ltc_fee < ltc_input_amount);
        pegin_amount = (uint64_t)(ltc_input_amount - ltc_fee);
    }

    CAmount recipient_amount = std::accumulate(
        recipients.cbegin(), recipients.cend(), CAmount(0),
        [](CAmount amount, const CRecipient& recipient) {
            return recipient.nAmount + amount;
        }
    );

    CAmount change_amount = (pegin_amount.value_or(0) + GetMWEBInputAmount(selected_coins)) - (recipient_amount + mweb_fee);

    libmw::MWEBAddress change_address = mweb_wallet->GetStealthAddress(libmw::CHANGE_INDEX);
    return libmw::MWEBRecipient{(uint64_t)change_amount, change_address};
}