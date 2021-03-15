#include <mweb/mweb_wallet.h>
#include <wallet/wallet.h>
#include <wallet/coincontrol.h>
#include <util/bip32.h>
#include <climits>

bool MWEB::Wallet::IsMine(const libmw::PubKey& spend_pubkey, uint32_t& index_out)
{
    while (m_pubkeys.size() < m_pWallet->GetHDChain().nMWEBIndexCounter) {
        assert(m_pubkeys.size() < ULONG_MAX);
        uint32_t pubkey_index = (uint32_t)m_pubkeys.size();

        libmw::MWEBAddress address = libmw::wallet::GetAddress(m_pWallet->GetMWWallet(), pubkey_index);
        CTxDestination dest = DecodeDestination(address);
        assert(dest.type() == typeid(MWEBDestination));

        m_pubkeys.insert({boost::get<MWEBDestination>(dest).spend_pubkey, pubkey_index});
    }
    
    auto iter = m_pubkeys.find(CPubKey(spend_pubkey.begin(), spend_pubkey.end()));
    if (iter != m_pubkeys.end()) {
        index_out = iter->second;
        return true;
    }

    return false;
}

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
        m_coins[coin.commitment] = coin;
    }
}

void MWEB::Wallet::DeleteCoins(const std::vector<libmw::Coin>& coins)
{
    WalletBatch batch(m_pWallet->GetDBHandle());
    for (const auto& coin : coins) {
        m_coins.erase(coin.commitment);
    }
}