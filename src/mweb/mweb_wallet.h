#pragma once

#include <amount.h>
#include <key.h>
#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/wallet/Coin.h>
#include <mw/models/wallet/StealthAddress.h>
#include <mw/wallet/Keychain.h>
#include <streams.h>
#include <util/strencodings.h>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <map>
#include <set>

class CWallet;

namespace MWEB {

class Wallet
{
public:
    Wallet(CWallet* pWallet)
        : m_pWallet(pWallet) {}

    const mw::Keychain::Ptr& GetKeychain() const;

    CExtKey GetHDKey(const std::string& bip32Path) const;
    bool GetCoin(const Commitment& output_commit, mw::Coin& coin) const;

    bool RewindOutput(
        const boost::variant<mw::Block::CPtr, mw::Transaction::CPtr>& parent,
        const Commitment& output_commit,
        mw::Coin& coin
    );
    bool IsMine(const StealthAddress& address) const;
    StealthAddress GetStealthAddress(const uint32_t index);
    StealthAddress GenerateNewAddress();

    void LoadToWallet(const mw::Coin& coin);
    void DeleteCoins(const std::vector<mw::Coin>& coins);

private:
    CWallet* m_pWallet;
    mutable mw::Keychain::Ptr m_keychain;
    std::map<Commitment, mw::Coin> m_coins;
};

struct WalletTxInfo
{
    // When connecting a block, if an output is found that belongs to us,
    // we check if we have a CWalletTx that created it.
    // If none is found, then we assume it is a newly received coin,
    // so we create an empty transaction and store the received coin here.
    boost::optional<mw::Coin> received_coin;

    // When connecting a block, if one of the wallet's coins is spent,
    // we check if we have a CWalletTx that spent it.
    // If none is found, then we assume it was spent by another wallet,
    // so we create an empty Transaction and store the spent commitment here.
    boost::optional<Commitment> spent_input;

    uint256 hash;

    WalletTxInfo()
        : received_coin(boost::none), spent_input(boost::none), hash() { }
    WalletTxInfo(mw::Coin received)
        : received_coin(std::move(received)), spent_input(boost::none)
    {
        hash = SerializeHash(*this);
    }
    WalletTxInfo(Commitment spent)
        : received_coin(boost::none), spent_input(std::move(spent))
    {
        hash = SerializeHash(*this);
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        if (ser_action.ForRead()) {
            bool received = false;
            READWRITE(received);

            if (received) {
                mw::Coin coin;
                READWRITE(coin);
                received_coin = boost::make_optional<mw::Coin>(std::move(coin));
            } else {
                Commitment input_commit;
                READWRITE(input_commit);
                spent_input = boost::make_optional<Commitment>(std::move(input_commit));
            }

            hash = SerializeHash(*this);
        } else {
            bool received = received_coin;
            READWRITE(received);

            if (received) {
                READWRITE(*received_coin);
            } else {
                READWRITE(*spent_input);
            }
        }
    }

    static WalletTxInfo FromHex(const std::string& str)
    {
        std::vector<uint8_t> bytes = ParseHex(str);
        CDataStream stream((const char*)bytes.data(), (const char*)bytes.data() + bytes.size(), SER_DISK, PROTOCOL_VERSION);

        WalletTxInfo wtx_info;
        stream >> wtx_info;
        return wtx_info;
    }

    std::string ToHex() const
    {
        CDataStream stream(SER_DISK, PROTOCOL_VERSION);
        stream << *this;
        return HexStr(std::vector<uint8_t>{stream.begin(), stream.end()});
    }

    const uint256& GetHash() const { return hash; }
};

} // namespace MWEB