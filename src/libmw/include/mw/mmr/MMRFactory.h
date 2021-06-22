#pragma once

#include <mw/common/BitSet.h>
#include <mw/common/Macros.h>
#include <mw/mmr/MMR.h>
#include <mw/mmr/MMRInfo.h>
#include <mw/mmr/PruneList.h>
#include <mw/models/crypto/Hash.h>
#include <mw/interfaces/db_interface.h>

class MMRFactory
{
public:
    static MMR::Ptr Build(
        const char prefix,
        const std::shared_ptr<mw::DBWrapper>& pDBWrapper,
        const std::unique_ptr<mw::DBBatch>& pBatch,
        const PruneList::Ptr& pPruneList,
        const MMRInfo& mmr_info,
        const FilePath& data_dir,
        const BitSet& unspent_leaf_indices,
        const std::vector<mmr::Leaf>& unspent_leaves,
        const std::vector<mw::Hash>& pruned_parent_hashes
    );

    static std::vector<mw::Hash> CalcHashes(
        const BitSet& unspent_leaf_indices,
        const std::vector<mmr::Leaf>& unspent_leaves,
        const std::vector<mw::Hash>& pruned_parent_hashes
    );
};