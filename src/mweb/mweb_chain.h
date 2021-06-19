#pragma once

#include <interfaces/chain.h>
#include <mw/interfaces/chain_interface.h>
#include <primitives/block.h>
#include <util/memory.h>

#include <stdexcept>

namespace MWEB {

class ChainIterator : public mw::IChainIterator
{
public:
    ChainIterator(interfaces::Chain& chain)
        : m_chain(chain), m_locked(chain.lock()), m_height(0) {}

    void SeekToFirstMWEB() noexcept
    {
        while (Valid()) {
            CBlock block;
            if (m_chain.findBlock(m_locked->getBlockHash(m_height), &block) && !block.mweb_block.IsNull()) {
                break;
            }

            Next();
        }
    }

    void Next() noexcept final { ++m_height; }
    bool Valid() const noexcept final { return m_locked->getHeight() != nullopt && m_height <= *m_locked->getHeight(); }

    uint64_t GetHeight() const final { return m_height; }

    mw::Header::CPtr GetHeader() const final
    {
        CBlock block;
        if (m_chain.findBlock(m_locked->getBlockHash(m_height), &block) && !block.mweb_block.IsNull()) {
            return block.mweb_block.m_block.GetHeader();
        }

        throw std::runtime_error("MWEB header not found");
    }

    mw::Block::CPtr GetBlock() const final
    {
        CBlock block;
        if (m_chain.findBlock(m_locked->getBlockHash(m_height), &block) && !block.mweb_block.IsNull()) {
            return block.mweb_block.m_block;
        }

        throw std::runtime_error("MWEB block not found");
    }

private:
    interfaces::Chain& m_chain;
    std::unique_ptr<interfaces::Chain::Lock> m_locked;
    int m_height;
};

class Chain : public mw::IChain
{
public:
    Chain(interfaces::Chain& chain)
        : m_chain(chain) {}

    std::unique_ptr<mw::IChainIterator> NewIterator() final
    {
        auto pIter = MakeUnique<MWEB::ChainIterator>(m_chain);
        pIter->SeekToFirstMWEB();
        return pIter;
    }

private:
    interfaces::Chain& m_chain;
};

} // namespace MWEB