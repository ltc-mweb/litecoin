#pragma once

#include <libmw/libmw.h>
#include <mw/models/wallet/StealthAddress.h>

class Transform
{
public:
    static libmw::MWEBAddress Address(const StealthAddress& address)
    {
        return std::make_pair(address.GetScanPubKey().array(), address.GetSpendPubKey().array());
    }

    static StealthAddress Address(const libmw::MWEBAddress& address)
    {
        return StealthAddress(
            PublicKey{address.first},
            PublicKey{address.second}
        );
    }
};