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

TxType GetTxType(const std::vector<CRecipient>& recipients, const std::vector<CInputCoin>& input_coins);

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