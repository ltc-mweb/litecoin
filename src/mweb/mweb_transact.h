#pragma once

#include <libmw/libmw.h>
#include <mweb/mweb_models.h>
#include <wallet/coinselection.h>
#include <wallet/wallet.h>
#include <boost/optional.hpp>
#include <numeric>

namespace MWEB {

enum class TxType {
    LTC_TO_LTC,
    MWEB_TO_MWEB,
    PEGIN,
    PEGOUT // NOTE: It's possible pegout transactions will also have pegins, but they will still be classified as PEGOUT
};

static TxType GetTxType(const std::vector<CRecipient>& recipients, const std::vector<CInputCoin>& input_coins)
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

class Transact
{
public:
    static bool CreateTx(
        const libmw::IWallet::Ptr& mweb_wallet,
        CMutableTransaction& transaction,
        const std::vector<CInputCoin>& selected_coins,
        const std::vector<CRecipient>& recipients,
        const CAmount& mweb_fee,
        const boost::optional<CAmount>& mweb_change
    );

private:
    static std::vector<libmw::Commitment> GetInputCommits(const std::vector<CInputCoin>& inputs);
    static CAmount GetMWEBInputAmount(const std::vector<CInputCoin>& inputs);
    static CAmount GetMWEBRecipientAmount(const std::vector<CRecipient>& recipients);
};

}