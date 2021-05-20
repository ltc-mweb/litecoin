#pragma once

#include <mw/mmr/LeafSet.h>
#include <mw/models/crypto/Commitment.h>
#include <mw/exceptions/NotFoundException.h>
#include <test_framework/TestUtil.h>

class TestLeafSet
{
public:
    using Ptr = std::shared_ptr<TestLeafSet>;

    struct BlockInfo
    {
        std::vector<Commitment> inputs;
        std::vector<Commitment> outputs;
    };

    static TestLeafSet::Ptr Create(const std::vector<BlockInfo>& blocks)
    {
        FilePath path = test::TestUtil::GetTempDir();
        auto pLeafSet = mmr::LeafSet::Open(path, 0);

        auto pTestLeafSet = std::shared_ptr<TestLeafSet>(new TestLeafSet(path, pLeafSet));

        std::unordered_map<Commitment, mmr::LeafIndex> unspentLeaves;
        for (const BlockInfo& block : blocks) {
            for (const Commitment& input : block.inputs) {
                auto iter = unspentLeaves.find(input);
                if (iter == unspentLeaves.cend()) {
                    ThrowNotFound_F("Unspent leaf not found with input commitment {}", input);
                }

                pLeafSet->Remove(iter->second);
            }

            for (const Commitment& output : block.outputs) {
                auto iter = unspentLeaves.find(output);
                if (iter != unspentLeaves.cend()) {
                    ThrowNotFound_F("Unspent leaf not found with output commitment {}", output);
                }

                unspentLeaves.insert({ output, pTestLeafSet->m_nextLeafIdx });
                pLeafSet->Add(pTestLeafSet->m_nextLeafIdx);
                pTestLeafSet->m_nextLeafIdx = pTestLeafSet->m_nextLeafIdx.Next();
            }
        }

        return pTestLeafSet;
    }

    ~TestLeafSet()
    {
        m_pLeafSet.reset();
        m_path.Remove();
    }

    mw::Hash Root() const
    {
        return m_pLeafSet->Root();
    }

private:
    TestLeafSet(const FilePath& path, const mmr::LeafSet::Ptr& pLeafSet)
        : m_path(path), m_pLeafSet(pLeafSet), m_nextLeafIdx(mmr::LeafIndex::At(0)){ }

    FilePath m_path;
    mmr::LeafSet::Ptr m_pLeafSet;
    mmr::LeafIndex m_nextLeafIdx;
};