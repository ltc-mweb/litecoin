#pragma once

#include <mw/common/Macros.h>
#include <mw/models/wallet/StealthAddress.h>

MW_NAMESPACE

//struct PegOutRecipient {
//    uint64_t amount;
//
//    /// <summary>
//    /// 4-42 bytes
//    /// </summary>
//    std::vector<uint8_t> scriptPubKey;
//};

struct Recipient {
    uint64_t amount;
    ::StealthAddress address;
};

END_NAMESPACE