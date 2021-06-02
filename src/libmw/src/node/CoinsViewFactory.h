#pragma once

#include <mw/common/BitSet.h>
#include <mw/file/FilePath.h>
#include <mw/mmr/MMR.h>
#include <mw/mmr/MMRInfo.h>
#include <mw/mmr/PruneList.h>
#include <mw/models/block/Header.h>
#include <mw/models/tx/UTXO.h>
#include <mw/node/CoinsView.h>
#include <mw/interfaces/chain_interface.h>
#include <mw/interfaces/db_interface.h>
#include <functional>

class CoinsViewFactory
{
public:
    static mw::CoinsViewDB::Ptr CreateDBView(
        const std::shared_ptr<mw::DBWrapper>& pDBWrapper,
        const mw::IChain::Ptr& pChain,
        const FilePath& chainDir,
        const mw::Header::CPtr& pStateHeader,
        const std::vector<UTXO::CPtr>& utxos,
        const std::vector<Kernel>& kernels,
        const BitSet& leafset,
        const std::vector<mw::Hash>& pruned_parent_hashes
    );

private:
    static mmr::MMR::Ptr BuildAndValidateKernelMMR(
        const std::shared_ptr<mw::DBWrapper>& pDBWrapper,
        const std::unique_ptr<mw::DBBatch>& pBatch,
        const MMRInfo& mmr_info,
        const mw::IChain::Ptr& pChain,
        const FilePath& chainDir,
        const mw::Header::CPtr& pStateHeader,
        const std::vector<Kernel>& kernels
    );

    static mmr::MMR::Ptr BuildAndValidateOutputMMR(
        const std::shared_ptr<mw::DBWrapper>& pDBWrapper,
        const std::unique_ptr<mw::DBBatch>& pBatch,
        const MMRInfo& mmr_info,
        const FilePath& chainDir,
        const mw::Header::CPtr& pStateHeader,
        const std::vector<UTXO::CPtr>& utxos,
        const BitSet& leafset,
        const mmr::PruneList::Ptr& pPruneList,
        const std::vector<mw::Hash>& pruned_parent_hashes
    );
};