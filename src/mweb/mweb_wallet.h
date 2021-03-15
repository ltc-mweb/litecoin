#pragma once

#include <libmw/libmw.h>
#include <amount.h>
#include <pubkey.h>
#include <map>
#include <set>

class CWallet;

namespace MWEB {

class Wallet : public libmw::IWallet
{
public:
    Wallet(CWallet* pWallet)
        : m_pWallet(pWallet) {}

    bool IsMine(const libmw::PubKey& spend_pubkey, uint32_t& index_out) final;
    libmw::PrivateKey GetHDKey(const std::string& bip32Path) const final;
    bool GetCoin(const libmw::Commitment& output_commit, libmw::Coin& coin) const final;

    void LoadToWallet(const libmw::Coin& coin);
    void AddCoins(const std::vector<libmw::Coin>& coins);
    void DeleteCoins(const std::vector<libmw::Coin>& coins);

private:
    CWallet* m_pWallet;
    std::map<libmw::Commitment, libmw::Coin> m_coins;
    std::map<CPubKey, uint32_t> m_pubkeys;
};

} // namespace MWEB