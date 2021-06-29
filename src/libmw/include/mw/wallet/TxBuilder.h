#pragma once

#include <mw/models/tx/Transaction.h>
#include <mw/models/tx/PegInCoin.h>
#include <mw/models/tx/PegOutCoin.h>
#include <mw/models/wallet/Coin.h>
#include <mw/models/wallet/Recipient.h>

class TxBuilder
{
    struct Outputs
    {
        BlindingFactor total_blind;
        SecretKey total_key;
        std::vector<Output> outputs;
    };

public:
    static mw::Transaction::CPtr BuildTx(
        const std::vector<mw::Coin>& input_coins,
        const std::vector<mw::Recipient>& recipients,
        const std::vector<PegOutCoin>& pegouts,
        const boost::optional<CAmount>& pegin_amount,
        const CAmount fee
    );

private:
    static Outputs CreateOutputs(const std::vector<mw::Recipient>& recipients);
};