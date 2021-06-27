#pragma once

#include <mw/models/wallet/Coin.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/tx/Input.h>
#include <mw/crypto/Hasher.h>
#include <mw/crypto/Pedersen.h>
#include <amount.h>
#include <numeric>

class WalletUtil
{
public:
    static std::vector<BlindingFactor> GetBlindingFactors(const std::vector<mw::Coin>& coins)
    {
        std::vector<BlindingFactor> blinds;

        std::transform(
            coins.cbegin(), coins.cend(), std::back_inserter(blinds),
            [](const mw::Coin& coin) {
                assert(!!coin.blind);
                return BlindingFactor{ Pedersen::BlindSwitch(coin.blind.value(), coin.amount) };
            }
        );

        return blinds;
    }

    static std::vector<SecretKey> GetKeys(const std::vector<mw::Coin>& coins)
    {
        std::vector<SecretKey> keys;

        std::transform(
            coins.cbegin(), coins.cend(), std::back_inserter(keys),
            [](const mw::Coin& coin) {
                assert(!!coin.key);
                return coin.key.value();
            }
        );

        return keys;
    }
    
    static CAmount TotalAmount(const std::vector<mw::Coin>& coins)
    {
        return std::accumulate(
            coins.cbegin(), coins.cend(), (CAmount)0,
            [](CAmount total, const mw::Coin& coin) { return total + coin.amount; }
        );
    }

    static std::vector<Input> SignInputs(const std::vector<mw::Coin>& input_coins)
    {
        std::vector<Input> inputs;
        std::transform(
            input_coins.cbegin(), input_coins.cend(), std::back_inserter(inputs),
            [](const mw::Coin& input_coin) {
                assert(!!input_coin.key);
                return Input(
                    input_coin.commitment,
                    PublicKey::From(input_coin.key.value()),
                    Schnorr::Sign(input_coin.key.value().data(), InputMessage())
                );
            }
        );

        return inputs;
    }
};