// Copyright(C) 2011 - 2020 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LITECOIN_MWEB_ADDRESS_H
#define LITECOIN_MWEB_ADDRESS_H

#include <amount.h>
#include <libmw/libmw.h>
#include <pubkey.h>
#include <serialize.h>

#include <memory>
#include <vector>

namespace MWEB {

struct StealthAddress {
    CPubKey scan_pubkey;
    CPubKey spend_pubkey;

    CKeyID GetID() const { return spend_pubkey.GetID(); }

    static StealthAddress From(const libmw::MWEBAddress& address)
    {
        return StealthAddress{
            CPubKey(address.first.begin(), address.first.end()),
            CPubKey(address.second.begin(), address.second.end())};
    }

    libmw::MWEBAddress to_libmw() const noexcept
    {
        libmw::MWEBAddress address;
        std::copy(scan_pubkey.begin(), scan_pubkey.end(), address.first.begin());
        std::copy(spend_pubkey.begin(), spend_pubkey.end(), address.second.begin());
        return address;
    }

    friend bool operator==(const StealthAddress& a1, const StealthAddress& a2)
    {
        return a1.scan_pubkey == a2.scan_pubkey && a1.spend_pubkey == a2.spend_pubkey;
    }

    friend bool operator<(const StealthAddress& a1, const StealthAddress& a2)
    {
        if (a1.scan_pubkey < a2.scan_pubkey) return true;
        if (a1.scan_pubkey != a2.scan_pubkey) return false;
        return a2.spend_pubkey < a2.spend_pubkey;
    }
};

} // namespace MWEB

#endif // LITECOIN_MWEB_ADDRESS_H