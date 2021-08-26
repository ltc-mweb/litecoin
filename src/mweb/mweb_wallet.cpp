#include <mweb/mweb_wallet.h>
#include <wallet/wallet.h>
#include <wallet/coincontrol.h>
#include <util/bip32.h>

using namespace MWEB;

std::vector<mw::Coin> Wallet::RewindOutputs(const CTransaction& tx)
{
    std::vector<mw::Coin> coins;
    for (const CTxOutput& txout : tx.GetOutputs()) {
        if (txout.IsMWEB()) {
            mw::Coin mweb_coin;
            if (RewindOutput(tx.mweb_tx.m_transaction, txout.GetCommitment(), mweb_coin)) {
                coins.push_back(mweb_coin);
            }
        }
    }

    return coins;
}

bool Wallet::RewindOutput(const boost::variant<mw::Block::CPtr, mw::Transaction::CPtr>& parent,
        const Commitment& output_commit, mw::Coin& coin)
{
    if (GetCoin(output_commit, coin)) {
        return true;
    }

    mw::Keychain::Ptr keychain = GetKeychain();
    if (!keychain) {
        return false;
    }

    bool rewound = false;
    if (parent.type() == typeid(mw::Block::CPtr)) {
        const mw::Block::CPtr& block = boost::get<mw::Block::CPtr>(parent);
        for (const Output& output : block->GetOutputs()) {
            if (output.GetCommitment() == output_commit) {
                rewound = keychain->RewindOutput(output, coin);
                break;
            }
        }
    } else {
        const mw::Transaction::CPtr& tx = boost::get<mw::Transaction::CPtr>(parent);
        for (const Output& output : tx->GetOutputs()) {
            if (output.GetCommitment() == output_commit) {
                rewound = keychain->RewindOutput(output, coin);
                break;
            }
        }
    }

    if (rewound) {
        m_coins[coin.commitment] = coin;
        WalletBatch(m_pWallet->GetDatabase()).WriteCoin(coin);
    }

    return rewound;
}

StealthAddress Wallet::GetStealthAddress(const uint32_t index)
{
    mw::Keychain::Ptr keychain = GetKeychain();
    assert(keychain != nullptr);
    return keychain->GetStealthAddress(index);
}

void Wallet::LoadToWallet(const mw::Coin& coin)
{
    LogPrintf("Coin %s loaded\n", coin.commitment.ToHex());
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

mw::Keychain::Ptr Wallet::GetKeychain() const
{
    auto spk_man = m_pWallet->GetScriptPubKeyMan(OutputType::MWEB, false);
    return spk_man ? spk_man->GetMWEBKeychain() : nullptr;
}