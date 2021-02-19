#pragma once

#include <libmw/libmw.h>
#include <util/bip32.h>
#include <wallet/wallet.h>
#include <wallet/coincontrol.h>
#include <map>

namespace MWEB {

class Wallet : public libmw::IWallet
{
public:
    Wallet(CWallet* pWallet, interfaces::Chain* pChain)
        : m_pWallet(pWallet), m_pChain(pChain) {}

    libmw::PrivateKey GetHDKey(const std::string& bip32Path) const final
    {
        // Currently, MWEB only supports HD wallets
        if (!m_pWallet->IsHDEnabled()) {
            throw std::runtime_error(std::string(__func__) + ": MWEB only supports HD wallets");
        }

        // Parse bip32 keypath
        std::vector<uint32_t> keyPath;
        if (!ParseHDKeypath(bip32Path, keyPath)) {
            throw std::runtime_error(std::string(__func__) + ": failed to parse HD keypath - " + bip32Path);
        }

        // Retrieve the HD seed
        CKey seed;
        if (!m_pWallet->GetKey(m_pWallet->GetHDChain().seed_id, seed)) {
            throw std::runtime_error(std::string(__func__) + ": seed not found");
        }

        // Derive key from path
        CExtKey extKey;
        extKey.SetSeed(seed.begin(), seed.size());

        for (const uint32_t child : keyPath) {
            CExtKey tempKey;
            extKey.Derive(tempKey, child);
            extKey = tempKey;
        }

        // Create libmw::PrivateKey
        libmw::PrivateKey privateKey;
        std::copy(extKey.key.begin(), extKey.key.end(), privateKey.keyBytes.data());
        privateKey.bip32Path = bip32Path;
        return privateKey;
    }

    std::vector<libmw::Coin> ListCoins() const final
    {
        // MW: TODO - Return the map instead.
        std::vector<libmw::Coin> coins;
        std::transform(
            m_coins.cbegin(), m_coins.cend(),
            std::back_inserter(coins),
            [](const auto& entry) { return entry.second; });

        return coins;
    }

    void LoadToWallet(const libmw::Coin& coin)
    {
        m_coins[coin.commitment] = coin;
    }

    libmw::Coin GetCoin(const libmw::Commitment& output_commit) const final
    {
        auto iter = m_coins.find(output_commit);
        if (iter != m_coins.end()) {
            return iter->second;
        }

        throw std::runtime_error(std::string(__func__) + ": Could not find coin with matching output commit");
    }

    void AddCoins(const std::vector<libmw::Coin>& coins) final
    {
        WalletBatch batch(m_pWallet->GetDBHandle());
        for (const auto& coin : coins) {
            batch.WriteMWCoin(coin);
            m_coins[coin.commitment] = coin;
        }
    }

    void DeleteCoins(const std::vector<libmw::Coin>& coins) final
    {
        WalletBatch batch(m_pWallet->GetDBHandle());
        for (const auto& coin : coins) {
            batch.EraseMWCoin(coin.commitment);
            m_coins.erase(coin.commitment);
        }
    }

    std::vector<libmw::Coin> SelectCoins(
        const std::vector<libmw::Coin>& coins,
        const uint64_t amount) const final
    {
        std::vector<COutputCoin> vCoins;
        for (const libmw::Coin& coin : coins) {
            if (!m_pWallet->IsLockedCoin(coin.commitment)) {
                vCoins.push_back(m_pWallet->MakeOutputCoin(*m_pChain->lock(), coin));
            }
        }

        std::set<CInputCoin> setCoins;
        CAmount nValueIn;
        CCoinControl coinControl;
        CoinSelectionParams coin_selection_params;
        bool bnb_used;

        bool ok = m_pWallet->SelectCoins(vCoins, amount, setCoins, nValueIn, coinControl, coin_selection_params, bnb_used);
        if (!ok && bnb_used) {
            coin_selection_params.use_bnb = false;
            ok = m_pWallet->SelectCoins(vCoins, amount, setCoins, nValueIn, coinControl, coin_selection_params, bnb_used);
        }
        if (!ok) throw std::runtime_error(std::string(__func__) + ": insufficient funds");

        std::vector<libmw::Coin> result;
        std::transform(
            setCoins.cbegin(), setCoins.cend(),
            std::back_inserter(result),
            [](const CInputCoin& coin) { return *coin.mwCoin; });
        return result;
    }

    uint64_t GetDepthInActiveChain(const libmw::BlockHash& canonical_block_hash) const final
    {
        auto locked_chain = m_pWallet->chain().lock();

        return locked_chain->getBlockDepth(uint256(std::vector<uint8_t>{canonical_block_hash.begin(), canonical_block_hash.end()}));
    }

private:
    CWallet* m_pWallet;
    interfaces::Chain* m_pChain;
    std::map<libmw::Commitment, libmw::Coin> m_coins;
};

} // namespace MWEB