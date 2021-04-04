#pragma once

#include <amount.h>
#include <key.h>
#include <libmw/libmw.h>
#include <mweb/mweb_address.h>
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
    
    CExtKey GetHDKey(const std::string& bip32Path) const;
    bool GetCoin(const libmw::Commitment& output_commit, libmw::Coin& coin) const;

    bool RewindOutput(
        const boost::variant<libmw::BlockRef, libmw::TxRef>& parent,
        const libmw::Commitment& output_commit,
        libmw::Coin& coin
    );
    MWEB::StealthAddress GetStealthAddress(const uint32_t index);
    bool GenerateNewAddress(MWEB::StealthAddress& address);

    void LoadToWallet(const libmw::Coin& coin);
    void DeleteCoins(const std::vector<libmw::Coin>& coins);

private:
    libmw::KeychainRef GetKeychain();

    CWallet* m_pWallet;
    libmw::KeychainRef m_keychain;
    std::map<libmw::Commitment, libmw::Coin> m_coins;
};

struct WalletTxInfo
{
    // When connecting a block, if an output si found that belongs to us,
    // we check if we have a CWalletTx that created it.
    // If none is found, then we assume it is a newly received coin,
    // so we create an empty transaction and store the received coin here.
    boost::optional<libmw::Coin> received_coin;

    // When connecting a block, if one of the wallet's coins is spent,
    // we check if we have a CWalletTx that spent it.
    // If none is found, then we assume it was spent by another wallet,
    // so we create an empty Transaction and store the spent commitment here.
    boost::optional<libmw::Commitment> spent_input;

    uint256 hash;

    WalletTxInfo()
        : received_coin(boost::none), spent_input(boost::none), hash() { }
    WalletTxInfo(libmw::Coin received)
        : received_coin(std::move(received)), spent_input(boost::none)
    {
        hash = SerializeHash(*this);
    }
    WalletTxInfo(libmw::Commitment spent)
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
                std::vector<uint8_t> bytes;
                READWRITE(bytes);
                received_coin = libmw::DeserializeCoin(bytes);
            } else {
                libmw::Commitment input_commit;
                READWRITE(input_commit);
                spent_input = input_commit;
            }

            hash = SerializeHash(*this);
        } else {
            bool received = received_coin;
            READWRITE(received);

            if (received) {
                std::vector<uint8_t> bytes = libmw::SerializeCoin(*received_coin);
                READWRITE(bytes);
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