#include <mweb/mweb_transact.h>


bool MWEB::Transact::CreateTx(
    const libmw::IWallet::Ptr& mweb_wallet,
    CMutableTransaction& transaction,
    const std::vector<CInputCoin>& selected_coins,
    const std::vector<CRecipient>& recipients,
    const CAmount& mweb_fee,
    const boost::optional<CAmount>& mweb_change)
{
    TxType type = GetTxType(recipients, selected_coins);

    if (type == TxType::LTC_TO_LTC) {
        LogPrintf("MWEB::Transaction - LTC_TO_LTC\n");
        return true;
    }

    // Standard MWEB-to-MWEB transaction.
    if (type == TxType::MWEB_TO_MWEB) {
        LogPrintf("MWEB::Transaction - MWEB_TO_MWEB\n");
        assert(mweb_change);

        std::vector<libmw::Recipient> mweb_recipients;
        for (const CRecipient& recipient : recipients) {
            if (recipient.IsMWEB()) {
                libmw::MWEBRecipient mweb_recipient{
                    (uint64_t)recipient.nAmount,
                    recipient.GetMWEBAddress()};

                mweb_recipients.push_back(std::move(mweb_recipient));
            }
        }

        std::vector<libmw::Commitment> input_commits = GetInputCommits(selected_coins);

        // Add Change
        libmw::MWEBAddress change_address = libmw::wallet::GetAddress(mweb_wallet, libmw::CHANGE_INDEX);
        mweb_recipients.push_back(libmw::MWEBRecipient{(uint64_t)*mweb_change, change_address});

        transaction.m_mwtx = libmw::wallet::CreateTx(
            mweb_wallet,
            input_commits,
            mweb_recipients,
            boost::none,
            (uint64_t)mweb_fee);
        return true;
    }

    // Pegging-in. Change will be generated on the LTC side.
    if (type == TxType::PEGIN) {
        LogPrintf("MWEB::Transaction - PEGIN\n");
        assert(!mweb_change);

        std::vector<libmw::Recipient> mweb_recipients;
        for (const CRecipient& recipient : recipients) {
            if (recipient.IsMWEB()) {
                libmw::PegInRecipient pegin_recipient{
                    (uint64_t)recipient.nAmount,
                    recipient.GetMWEBAddress()};

                mweb_recipients.push_back(std::move(pegin_recipient));
            }
        }

        std::vector<libmw::Commitment> input_commits = GetInputCommits(selected_coins);
        CAmount pegin_amount = (GetMWEBRecipientAmount(recipients) + mweb_fee) - GetMWEBInputAmount(selected_coins);
        LogPrintf("Pegin amount: %d", pegin_amount);

        transaction.m_mwtx = libmw::wallet::CreateTx(
            mweb_wallet,
            input_commits,
            mweb_recipients,
            (uint64_t)pegin_amount,
            (uint64_t)mweb_fee);

        auto pegins = transaction.m_mwtx.GetPegIns();
        assert(pegins.size() == 1);


        CScript pegin_script;
        pegin_script << CScript::EncodeOP_N(Consensus::Mimblewimble::WITNESS_VERSION);
        pegin_script << std::vector<uint8_t>(pegins.front().commitment.cbegin(), pegins.front().commitment.cend());
        transaction.vout.back().nValue = pegin_amount;
        transaction.vout.back().scriptPubKey = pegin_script;

        return true;
    }

    if (type == TxType::PEGOUT) {
        LogPrintf("MWEB::Transaction - PEGOUT\n");
    }

    // MW: TODO - Peg-outs

    return false;
}

std::vector<libmw::Commitment> MWEB::Transact::GetInputCommits(const std::vector<CInputCoin>& inputs)
{
    std::vector<libmw::Commitment> input_commits;
    for (const auto& coin : inputs) {
        if (coin.IsMWEB()) {
            input_commits.push_back(coin.mwCoin->commitment);
        }
    }

    return input_commits;
}

CAmount MWEB::Transact::GetMWEBInputAmount(const std::vector<CInputCoin>& inputs)
{
    return std::accumulate(
        inputs.cbegin(), inputs.cend(), CAmount(0),
        [](CAmount amt, const CInputCoin& input) { return amt + (input.IsMWEB() ? input.GetAmount() : 0); });
}

CAmount MWEB::Transact::GetMWEBRecipientAmount(const std::vector<CRecipient>& recipients)
{
    return std::accumulate(
        recipients.cbegin(), recipients.cend(), CAmount(0),
        [](CAmount amt, const CRecipient& recipient) { return amt + (recipient.IsMWEB() ? recipient.nAmount : 0); });
}