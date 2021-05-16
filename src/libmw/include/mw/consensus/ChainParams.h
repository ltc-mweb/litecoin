#pragma once

#include <mw/common/Macros.h>
#include <string>

MW_NAMESPACE

class ChainParams
{
public:
    static void Initialize(const uint64_t pegin_maturity)
    {
        s_pegin_maturity = pegin_maturity;
    }

    static uint64_t GetPegInMaturity() noexcept { return s_pegin_maturity; }

private:
    inline static uint64_t s_pegin_maturity = 20;
};

END_NAMESPACE