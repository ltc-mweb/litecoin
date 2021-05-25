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

#include <mw/models/wallet/Coin.h>
#include <mw/consensus/Params.h>
#include <mw/models/block/Block.h>
#include <mw/models/block/BlockUndo.h>
#include <mw/models/tx/Transaction.h>
#include <mw/node/State.h>

#include <memory>
#include <cstdint>