#pragma once

#include <mw/common/Macros.h>
#include <mw/models/wallet/StealthAddress.h>

#include <boost/variant.hpp>

#include <cstdint>
#include <vector>

MW_NAMESPACE

struct PegOutRecipient {
    uint64_t amount;

    /// <summary>
    /// 4-42 bytes
    /// </summary>
    std::vector<uint8_t> scriptPubKey;
};

struct MWEBRecipient {
    uint64_t amount;
    StealthAddress address;
};

struct PegInRecipient {
    uint64_t amount;
    StealthAddress address;
};

typedef boost::variant<MWEBRecipient, PegInRecipient, PegOutRecipient> Recipient;

END_NAMESPACE