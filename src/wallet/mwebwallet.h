#pragma once

#include <libmw/libmw.h>
#include <util/bip32.h>
#include <wallet/wallet.h>

class MWWallet : public libmw::IWallet
{
public:
    MWWallet(CWallet* pWallet)
        : m_pWallet(pWallet), m_pBatch(std::make_unique<WalletBatch>(pWallet->GetDBHandle())) {}

    libmw::PrivateKey GenerateNewHDKey() final
    {
        // Currently, MWEB only supports HD wallets
        if (!m_pWallet->IsHDEnabled()) {
            throw std::runtime_error(std::string(__func__) + ": MWEB only supports HD wallets");
        }

        // Generate new HD key
        CKey key = m_pWallet->GenerateNewKey(*m_pBatch);

        // Create libmw::PrivateKey
        libmw::PrivateKey privateKey;
        std::copy(key.begin(), key.end(), privateKey.keyBytes.data());

        auto iter = m_pWallet->mapKeyMetadata.find(key.GetPubKey().GetID());
        assert(iter != m_pWallet->mapKeyMetadata.end());
        privateKey.bip32Path = iter->second.hdKeypath;

        return privateKey;
    }

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
        std::vector<libmw::Coin> coins;
        DBErrors errors = m_pBatch->FindMWCoins(coins);
        if (errors != DBErrors::LOAD_OK) {
            throw std::runtime_error(std::string(__func__) + ": FindMWCoins returned a DB error");
        }

        return coins;
    }

    void AddCoins(const std::vector<libmw::Coin>& coins) final
    {
        for (const auto& coin : coins) {
            m_pBatch->WriteMWCoin(coin);
        }
    }

    void DeleteCoins(const std::vector<libmw::Coin>& coins) final
    {
        for (const auto& coin : coins) {
            m_pBatch->EraseMWCoin(coin.commitment);
        }
    }

    std::vector<libmw::Coin> SelectCoins(
        const std::vector<libmw::Coin>& coins,
        const uint64_t amount) const final
    {
        std::vector<COutputCoin> vCoins;
        std::transform(
            coins.cbegin(), coins.cend(),
            std::back_inserter(vCoins),
            [](const libmw::Coin& coin) { return COutputCoin(coin); }
        );

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
            [](const CInputCoin& coin) { return *coin.mwCoin; }
        );
        return result;
    }

    uint64_t GetDepthInActiveChain(const libmw::BlockHash& canonical_block_hash) const final
    {
        auto locked_chain = m_pWallet->chain().lock();
        
        return locked_chain->getBlockDepth(uint256(std::vector<uint8_t>{canonical_block_hash.begin(), canonical_block_hash.end()}));
    }

private:
    CWallet* m_pWallet;
    std::unique_ptr<WalletBatch> m_pBatch;
};