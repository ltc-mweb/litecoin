#include <mweb/mweb_wallet.h>
#include <wallet/wallet.h>
#include <wallet/coincontrol.h>
#include <util/bip32.h>

using namespace MWEB;

bool Wallet::RewindOutput(const boost::variant<mw::Block::CPtr, mw::Transaction::CPtr>& parent,
        const Commitment& output_commit, mw::Coin& coin)
{
    if (GetCoin(output_commit, coin)) {
        return true;
    }

    bool rewound = false;
    if (parent.type() == typeid(mw::Block::CPtr)) {
        const mw::Block::CPtr& block = boost::get<mw::Block::CPtr>(parent);

        for (const Output& output : block->GetOutputs()) {
            if (output.GetCommitment() == output_commit) {
                rewound = GetKeychain()->RewindOutput(output, coin);
                break;
            }
        }
    } else {
        const mw::Transaction::CPtr& tx = boost::get<mw::Transaction::CPtr>(parent);
        for (const Output& output : tx->GetOutputs()) {
            if (output.GetCommitment() == output_commit) {
                rewound = GetKeychain()->RewindOutput(output, coin);
                break;
            }
        }
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

bool Wallet::IsMine(const StealthAddress& address) const
{
    return GetKeychain()->IsMine(address);
}

StealthAddress Wallet::GetStealthAddress(const uint32_t index)
{
    return GetKeychain()->GetStealthAddress(index);
}

StealthAddress Wallet::GenerateNewAddress()
{
    CHDChain hdChain = m_pWallet->GetHDChain();
    StealthAddress address = GetStealthAddress(hdChain.nMWEBIndexCounter++);
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

void Wallet::LoadToWallet(const mw::Coin& coin)
{
    m_coins[coin.commitment] = coin;
}

bool Wallet::GetCoin(const Commitment& output_commit, mw::Coin& coin) const
{
    auto iter = m_coins.find(output_commit);
    if (iter != m_coins.end()) {
        coin = iter->second;
        return true;
    }

    return false;
}

void Wallet::DeleteCoins(const std::vector<mw::Coin>& coins)
{
    for (const auto& coin : coins) {
        m_coins.erase(coin.commitment);
    }
}

const mw::Keychain::Ptr& Wallet::GetKeychain() const
{
    if (!m_keychain) {
        // Scan secret key
        SecretKey scan_secret(GetHDKey("m/1/0/100'").key.begin());

        // Spend secret key
        SecretKey spend_secret(GetHDKey("m/1/0/101'").key.begin());

        m_keychain = std::make_shared<mw::Keychain>(
            std::move(scan_secret),
            std::move(spend_secret),
            m_pWallet->GetHDChain().nMWEBIndexCounter
        );
    }

    return m_keychain;
}