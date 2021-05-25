#include <libmw/wallet.h>

#include <mw/models/tx/Transaction.h>
#include <mw/wallet/Keychain.h>
#include <mw/wallet/Transact.h>
#include <mw/wallet/Wallet.h>

LIBMW_NAMESPACE
WALLET_NAMESPACE

mw::Transaction::CPtr CreateTx(
    const std::vector<mw::Coin>& input_coins,
    const std::vector<libmw::Recipient>& recipients,
    const boost::optional<uint64_t>& pegin_amount,
    const uint64_t fee)
{
    std::vector<std::pair<uint64_t, StealthAddress>> receivers;
    std::vector<PegOutCoin> pegouts;

    for (const libmw::Recipient& recipient : recipients) {
        if (recipient.which() == 0) {
            const libmw::MWEBRecipient& mweb_recipient = boost::get<libmw::MWEBRecipient>(recipient);
            receivers.push_back(std::make_pair(mweb_recipient.amount, mweb_recipient.address));
        } else if (recipient.which() == 1) {
            const libmw::PegInRecipient& pegin_recipient = boost::get<libmw::PegInRecipient>(recipient);
            receivers.push_back(std::make_pair(pegin_recipient.amount, pegin_recipient.address));
        } else {
            const libmw::PegOutRecipient& pegout_recipient = boost::get<libmw::PegOutRecipient>(recipient);
            pegouts.push_back(PegOutCoin(pegout_recipient.amount, pegout_recipient.scriptPubKey));
        }
    }

    return Transact::CreateTx(input_coins, receivers, pegouts, pegin_amount, fee);
}

bool RewindBlockOutput(
    const mw::Keychain::Ptr& keychain,
    const mw::Block::CPtr& block,
    const Commitment& output_commit,
    mw::Coin& coin_out)
{
    assert(keychain != nullptr);
    assert(block != nullptr);

    Wallet wallet(keychain);

    for (const Output& output : block->GetOutputs()) {
        if (output.GetCommitment() == output_commit) {
            return wallet.RewindOutput(output, coin_out);
        }
    }

    return false;
}

bool RewindTxOutput(
    const mw::Keychain::Ptr& keychain,
    const mw::Transaction::CPtr& tx,
    const Commitment& output_commit,
    mw::Coin& coin_out)
{
    assert(keychain != nullptr);
    assert(tx != nullptr);

    Wallet wallet(keychain);

    for (const Output& output : tx->GetOutputs()) {
        if (output.GetCommitment() == output_commit) {
            return wallet.RewindOutput(output, coin_out);
        }
    }

    return false;
}

END_NAMESPACE // wallet
END_NAMESPACE // libmw