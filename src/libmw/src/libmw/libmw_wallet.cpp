#include <libmw/libmw.h>

#include <mw/models/tx/Transaction.h>
#include <mw/wallet/Keychain.h>
#include <mw/wallet/Transact.h>

LIBMW_NAMESPACE
WALLET_NAMESPACE

mw::Transaction::CPtr CreateTx(
    const std::vector<mw::Coin>& input_coins,
    const std::vector<mw::Recipient>& recipients,
    const boost::optional<uint64_t>& pegin_amount,
    const uint64_t fee)
{
    std::vector<std::pair<uint64_t, StealthAddress>> receivers;
    std::vector<PegOutCoin> pegouts;

    for (const mw::Recipient& recipient : recipients) {
        if (recipient.which() == 0) {
            const mw::MWEBRecipient& mweb_recipient = boost::get<mw::MWEBRecipient>(recipient);
            receivers.push_back(std::make_pair(mweb_recipient.amount, mweb_recipient.address));
        } else if (recipient.which() == 1) {
            const mw::PegInRecipient& pegin_recipient = boost::get<mw::PegInRecipient>(recipient);
            receivers.push_back(std::make_pair(pegin_recipient.amount, pegin_recipient.address));
        } else {
            const mw::PegOutRecipient& pegout_recipient = boost::get<mw::PegOutRecipient>(recipient);
            pegouts.push_back(PegOutCoin(pegout_recipient.amount, pegout_recipient.scriptPubKey));
        }
    }

    return Transact::CreateTx(input_coins, receivers, pegouts, pegin_amount, fee);
}

END_NAMESPACE // wallet
END_NAMESPACE // libmw