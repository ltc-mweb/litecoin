#pragma once

// Copyright (c) 2018-2020 David Burkett
// Copyright (c) 2020 The Litecoin Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "defs.h"
#include "node.h"
#include "wallet.h"
#include "miner.h"
#include "interfaces/chain_interface.h"
#include "interfaces/db_interface.h"

#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>

#include <memory>
#include <cstdint>

LIBMW_NAMESPACE

libmw::HeaderRef DeserializeHeader(const std::vector<uint8_t>& bytes);
std::vector<uint8_t> SerializeHeader(const libmw::HeaderRef& header);

libmw::BlockUndoRef DeserializeBlockUndo(const std::vector<uint8_t>& bytes);
std::vector<uint8_t> SerializeBlockUndo(const libmw::BlockUndoRef& blockUndo);

libmw::StateRef DeserializeState(const std::vector<uint8_t>& bytes);
std::vector<uint8_t> SerializeState(const libmw::StateRef& state);

END_NAMESPACE // libmw