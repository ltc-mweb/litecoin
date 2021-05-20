#include <mweb/mweb_wallet.h>
#include <wallet/wallet.h>
#include <wallet/coincontrol.h>
#include <util/bip32.h>

using namespace MWEB;

bool Wallet::RewindOutput(const boost::variant<mw::Block::CPtr, mw::Transaction::CPtr>& parent,
        const Commitment& output_commit, libmw::Coin& coin)
{
    if (GetCoin(output_commit, coin)) {
        return true;
    }

    bool rewound = false;
    if (parent.type() == typeid(mw::Block::CPtr)) {
        const mw::Block::CPtr& block = boost::get<mw::Block::CPtr>(parent);
        rewound = libmw::wallet::RewindBlockOutput(GetKeychain(), block, output_commit, coin);
    } else {
        const mw::Transaction::CPtr& tx = boost::get<mw::Transaction::CPtr>(parent);
        rewound = libmw::wallet::RewindTxOutput(GetKeychain(), tx, output_commit, coin);
    }

    if (rewound) {
        m_coins[coin.commitment] = coin;

        CHDChain hdChain = m_pWallet->GetHDChain();
        if (coin.address_index >= hdChain.nMWEBIndexCounter) {
            hdChain.nMWEBIndexCounter = (coin.address_index + 1);
            m_pWallet->SetHDChain(hdChain, false);
        }
    }

    return rewound;
}

MWEB::StealthAddress Wallet::GetStealthAddress(const uint32_t index)
{
    return MWEB::StealthAddress::From(GetKeychain().GetAddress(index));
}

MWEB::StealthAddress Wallet::GenerateNewAddress()
{
    CHDChain hdChain = m_pWallet->GetHDChain();
    MWEB::StealthAddress address = GetStealthAddress(hdChain.nMWEBIndexCounter++);
    m_pWallet->SetHDChain(hdChain, false);

    return address;
}

CExtKey Wallet::GetHDKey(const std::string& bip32Path) const
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

    return extKey;
}

void Wallet::LoadToWallet(const libmw::Coin& coin)
{
    m_coins[coin.commitment] = coin;
}

bool Wallet::GetCoin(const Commitment& output_commit, libmw::Coin& coin) const
{
    auto iter = m_coins.find(output_commit);
    if (iter != m_coins.end()) {
        coin = iter->second;
        return true;
    }

    return false;
}

void Wallet::DeleteCoins(const std::vector<libmw::Coin>& coins)
{
    for (const auto& coin : coins) {
        m_coins.erase(coin.commitment);
    }
}

libmw::KeychainRef Wallet::GetKeychain()
{
    if (!m_keychain.pKeychain) {
        // Scan secret key
        libmw::PrivateKey scan_secret;
        CKey scan_key = GetHDKey("m/1/0/100'").key;
        std::copy(scan_key.begin(), scan_key.end(), scan_secret.data());

        // Spend secret key
        libmw::PrivateKey spend_secret;
        CKey spend_key = GetHDKey("m/1/0/101'").key;
        std::copy(spend_key.begin(), spend_key.end(), spend_secret.data());

        m_keychain = libmw::wallet::LoadKeychain(scan_secret, spend_secret, m_pWallet->GetHDChain().nMWEBIndexCounter);
    }

    return m_keychain;
}