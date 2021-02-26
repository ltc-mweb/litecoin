#include <mweb/mweb_wallet.h>
#include <wallet/wallet.h>
#include <wallet/coincontrol.h>
#include <util/bip32.h>

libmw::PrivateKey MWEB::Wallet::GetHDKey(const std::string& bip32Path) const
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

void MWEB::Wallet::LoadToWallet(const libmw::Coin& coin)
{
    m_coins[coin.commitment] = coin;
}

bool MWEB::Wallet::GetCoin(const libmw::Commitment& output_commit, libmw::Coin& coin) const
{
    auto iter = m_coins.find(output_commit);
    if (iter != m_coins.end()) {
        coin = iter->second;
        return true;
    }

    return false;
}

void MWEB::Wallet::AddCoins(const std::vector<libmw::Coin>& coins)
{
    WalletBatch batch(m_pWallet->GetDBHandle());
    for (const auto& coin : coins) {
        batch.WriteMWCoin(coin);
        m_coins[coin.commitment] = coin;
    }
}

void MWEB::Wallet::DeleteCoins(const std::vector<libmw::Coin>& coins)
{
    WalletBatch batch(m_pWallet->GetDBHandle());
    for (const auto& coin : coins) {
        batch.EraseMWCoin(coin.commitment);
        m_coins.erase(coin.commitment);
    }
}