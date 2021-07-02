#pragma once

#include <mw/common/Macros.h>
#include <cstddef>
#include <cstdint>

MW_NAMESPACE

/// <summary>
/// Consensus parameters
/// Any change to these will cause a hardfork!
/// </summary>
static constexpr std::size_t MAX_BLOCK_WEIGHT = 21'000;
static constexpr std::size_t MAX_KERNEL_EXTRADATA_SIZE = 100;

END_NAMESPACE