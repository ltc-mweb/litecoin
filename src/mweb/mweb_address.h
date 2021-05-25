// Copyright(C) 2011 - 2020 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LITECOIN_MWEB_ADDRESS_H
#define LITECOIN_MWEB_ADDRESS_H

#include <amount.h>
#include <mw/models/wallet/StealthAddress.h>
#include <pubkey.h>
#include <serialize.h>

#include <memory>
#include <vector>

namespace MWEB {

// MW: TODO: Remove this struct and just use ::StealthAddress directly
struct StealthAddress {
    CPubKey scan_pubkey;
    CPubKey spend_pubkey;

    CKeyID GetID() const { return spend_pubkey.GetID(); }

    static StealthAddress From(const ::StealthAddress& address)
    {
        return StealthAddress{
            CPubKey(address.GetScanPubKey().vec()),
            CPubKey(address.GetSpendPubKey().vec())};
    }

    ::StealthAddress to_libmw() const noexcept
    {
        return ::StealthAddress(BigInt<33>(scan_pubkey.data()), BigInt<33>(spend_pubkey.data()));
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