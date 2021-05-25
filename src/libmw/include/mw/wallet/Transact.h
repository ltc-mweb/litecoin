#pragma once

#include <mw/models/tx/Transaction.h>
#include <mw/models/tx/PegInCoin.h>
#include <mw/models/tx/PegOutCoin.h>
#include <mw/models/wallet/Coin.h>
#include <mw/models/wallet/StealthAddress.h>

class Transact
{
    struct Outputs
    {
        BlindingFactor total_blind;
        BlindingFactor total_key;
        std::vector<Output> outputs;
    };

public:
    static mw::Transaction::CPtr CreateTx(
        const std::vector<mw::Coin>& input_coins,
        const std::vector<std::pair<uint64_t, StealthAddress>>& recipients,
        const std::vector<PegOutCoin>& pegouts,
        const boost::optional<uint64_t>& pegin_amount,
        const uint64_t fee
    );

private:
    static Outputs CreateOutputs(const std::vector<std::pair<uint64_t, StealthAddress>>& recipients);
};