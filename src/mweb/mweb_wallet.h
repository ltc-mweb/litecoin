#pragma once

#include <libmw/libmw.h>
#include <interfaces/chain.h>
#include <amount.h>
#include <map>

class CWallet;

namespace MWEB {

struct WalletTxInput
{
    libmw::Commitment commitment;
    boost::optional<CAmount> amount;
    bool mine;
};

struct WalletTxOutput
{
    libmw::Commitment commitment;
    boost::optional<CAmount> amount;
    boost::optional<libmw::MWEBAddress> address;
    bool mine;
};

struct WalletTx
{
    std::vector<WalletTxInput> inputs;
    std::vector<WalletTxOutput> outputs;

    // MW: TODO - Implement
    static WalletTx FromHex(const std::string& hex) { return {}; }
    std::string ToHex() const { return ""; }
};

class Wallet : public libmw::IWallet
{
public:
    Wallet(CWallet* pWallet, interfaces::Chain* pChain)
        : m_pWallet(pWallet), m_pChain(pChain) {}

    libmw::PrivateKey GetHDKey(const std::string& bip32Path) const final;

    std::vector<libmw::Coin> ListCoins() const final;
    void LoadToWallet(const libmw::Coin& coin);
    bool GetCoin(const libmw::Commitment& output_commit, libmw::Coin& coin) const final;
    void AddCoins(const std::vector<libmw::Coin>& coins) final;
    void DeleteCoins(const std::vector<libmw::Coin>& coins) final;

    std::vector<libmw::Coin> SelectCoins(
        const std::vector<libmw::Coin>& coins,
        const uint64_t amount
    ) const final;

    uint64_t GetDepthInActiveChain(const libmw::BlockHash& canonical_block_hash) const final;

private:
    CWallet* m_pWallet;
    interfaces::Chain* m_pChain;
    std::map<libmw::Commitment, libmw::Coin> m_coins;
};

} // namespace MWEB