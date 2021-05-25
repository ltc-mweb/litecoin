#pragma once

#include "defs.h"

#include <boost/variant.hpp>
#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/wallet/Coin.h>
#include <mw/models/wallet/StealthAddress.h>
#include <mw/wallet/Keychain.h>

#include <boost/optional.hpp>

LIBMW_NAMESPACE

struct PegOutRecipient
{
    uint64_t amount;

    /// <summary>
    /// 4-42 bytes
    /// </summary>
    std::vector<uint8_t> scriptPubKey;
};

struct MWEBRecipient
{
    uint64_t amount;
    StealthAddress address;
};

struct PegInRecipient
{
    uint64_t amount;
    StealthAddress address;
};

typedef boost::variant<MWEBRecipient, PegInRecipient, PegOutRecipient> Recipient;

WALLET_NAMESPACE

mw::Transaction::CPtr CreateTx(
    const std::vector<mw::Coin>& input_coins,
    const std::vector<libmw::Recipient>& recipients,
    const boost::optional<uint64_t>& pegin_amount,
    const uint64_t fee
);

bool RewindBlockOutput(
    const mw::Keychain::Ptr& keychain,
    const mw::Block::CPtr& block,
    const Commitment& output_commit,
    mw::Coin& coin_out
);

bool RewindTxOutput(
    const mw::Keychain::Ptr& keychain,
    const mw::Transaction::CPtr& tx,
    const Commitment& output_commit,
    mw::Coin& coin_out
);

END_NAMESPACE // wallet
END_NAMESPACE // libmw