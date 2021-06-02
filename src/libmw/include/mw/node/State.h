#pragma once

#include <mw/common/BitSet.h>
#include <mw/models/block/Header.h>
#include <mw/models/tx/Kernel.h>
#include <mw/models/tx/UTXO.h>
#include <mw/traits/Serializable.h>
#include <serialize.h>

MW_NAMESPACE

struct State
{
    mw::Hash header_hash;
    std::vector<Kernel> kernels;
    BitSet leafset;
    std::vector<UTXO::CPtr> utxos;
    std::vector<Hash> pruned_parent_hashes;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(header_hash);
        READWRITE(kernels);
        READWRITE(leafset);
        //READWRITE(utxos);
        READWRITE(pruned_parent_hashes);
    }
};

END_NAMESPACE