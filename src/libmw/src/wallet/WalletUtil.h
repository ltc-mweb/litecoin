#pragma once

#include <mw/models/wallet/Coin.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/tx/Input.h>
#include <mw/crypto/Crypto.h>
#include <mw/crypto/Hasher.h>
#include <numeric>

class WalletUtil
{
public:
    static std::vector<BlindingFactor> GetBlindingFactors(const std::vector<mw::Coin>& coins)
    {
        std::vector<BlindingFactor> blinds;

        std::transform(
            coins.cbegin(), coins.cend(),
            std::back_inserter(blinds),
            [](const mw::Coin& coin) {
                assert(!!coin.blind);
                return BlindingFactor{ Crypto::BlindSwitch(coin.blind.value(), coin.amount) };
            }
        );

        return blinds;
    }

    static std::vector<BlindingFactor> GetKeys(const std::vector<mw::Coin>& coins)
    {
        std::vector<BlindingFactor> keys;

        std::transform(
            coins.cbegin(), coins.cend(),
            std::back_inserter(keys),
            [](const mw::Coin& coin) {
                assert(!!coin.key);
                return BlindingFactor(coin.key.value());
            }
        );

        return keys;
    }
    
    static uint64_t TotalAmount(const std::vector<mw::Coin>& coins)
    {
        return std::accumulate(
            coins.cbegin(), coins.cend(), (uint64_t)0,
            [](uint64_t total, const mw::Coin& coin) { return total + coin.amount; }
        );
    }

    static std::vector<Input> SignInputs(const std::vector<mw::Coin>& input_coins)
    {
        std::vector<Input> inputs;
        std::transform(
            input_coins.cbegin(), input_coins.cend(),
            std::back_inserter(inputs),
            [](const mw::Coin& input_coin) {
                assert(!!input_coin.key);

                PublicKey pubkey = Crypto::CalculatePublicKey(input_coin.key.value().GetBigInt());
                Signature sig = Schnorr::Sign(input_coin.key.value().data(), InputMessage());
                return Input(input_coin.commitment, std::move(pubkey), std::move(sig));
            }
        );

        return inputs;
    }
};