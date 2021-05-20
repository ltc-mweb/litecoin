#include <libmw/wallet.h>

#include "Transformers.h"

#include <mw/models/tx/Transaction.h>
#include <mw/wallet/Keychain.h>
#include <mw/wallet/Transact.h>
#include <mw/wallet/Wallet.h>

LIBMW_NAMESPACE

libmw::MWEBAddress KeychainRef::GetAddress(const uint32_t index)
{
    assert(pKeychain != nullptr);

    return Transform::Address(pKeychain->GetStealthAddress(index));
}

WALLET_NAMESPACE

libmw::KeychainRef LoadKeychain(
    const libmw::PrivateKey& scan_key,
    const libmw::PrivateKey& spend_key,
    const uint32_t address_index_counter)
{
    auto pKeychain = std::make_shared<mw::Keychain>(
        scan_key,
        spend_key,
        address_index_counter
    );
    return libmw::KeychainRef{ pKeychain };
}

mw::Transaction::CPtr CreateTx(
    const std::vector<libmw::Coin>& input_coins,
    const std::vector<libmw::Recipient>& recipients,
    const boost::optional<uint64_t>& pegin_amount,
    const uint64_t fee)
{
    std::vector<std::pair<uint64_t, StealthAddress>> receivers;
    std::vector<PegOutCoin> pegouts;

    for (const libmw::Recipient& recipient : recipients) {
        if (recipient.which() == 0) {
            const libmw::MWEBRecipient& mweb_recipient = boost::get<libmw::MWEBRecipient>(recipient);
            receivers.push_back(std::make_pair(mweb_recipient.amount, Transform::Address(mweb_recipient.address)));
        } else if (recipient.which() == 1) {
            const libmw::PegInRecipient& pegin_recipient = boost::get<libmw::PegInRecipient>(recipient);
            receivers.push_back(std::make_pair(pegin_recipient.amount, Transform::Address(pegin_recipient.address)));
        } else {
            const libmw::PegOutRecipient& pegout_recipient = boost::get<libmw::PegOutRecipient>(recipient);
            pegouts.push_back(PegOutCoin(pegout_recipient.amount, pegout_recipient.scriptPubKey));
        }
    }

    return Transact::CreateTx(input_coins, receivers, pegouts, pegin_amount, fee);
}

bool RewindBlockOutput(
    const libmw::KeychainRef& keychain,
    const mw::Block::CPtr& block,
    const Commitment& output_commit,
    libmw::Coin& coin_out)
{
    assert(keychain.pKeychain != nullptr);
    assert(block != nullptr);

    Wallet wallet(keychain.pKeychain);

    for (const Output& output : block->GetOutputs()) {
        if (output.GetCommitment() == output_commit) {
            return wallet.RewindOutput(output, coin_out);
        }
    }

    return false;
}

bool RewindTxOutput(
    const libmw::KeychainRef& keychain,
    const mw::Transaction::CPtr& tx,
    const Commitment& output_commit,
    libmw::Coin& coin_out)
{
    assert(keychain.pKeychain != nullptr);
    assert(tx != nullptr);

    Wallet wallet(keychain.pKeychain);

    for (const Output& output : tx->GetOutputs()) {
        if (output.GetCommitment() == output_commit) {
            return wallet.RewindOutput(output, coin_out);
        }
    }

    return false;
}

END_NAMESPACE // wallet
END_NAMESPACE // libmw