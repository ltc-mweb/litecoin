#pragma once

#include <mw/common/Macros.h>
#include <mw/common/Traits.h>
#include <mw/models/tx/Output.h>
#include <mw/mmr/LeafIndex.h>
#include <serialize.h>

class UTXO : public Traits::ISerializable
{
public:
    using CPtr = std::shared_ptr<const UTXO>;

    UTXO() : m_blockHeight(0), m_leafIdx(), m_output() { }
    UTXO(const int32_t blockHeight, mmr::LeafIndex leafIdx, Output output)
        : m_blockHeight(blockHeight), m_leafIdx(std::move(leafIdx)), m_output(std::move(output)) { }

    int32_t GetBlockHeight() const noexcept { return m_blockHeight; }
    const mmr::LeafIndex& GetLeafIndex() const noexcept { return m_leafIdx; }
    const Output& GetOutput() const noexcept { return m_output; }
    OutputId ToOutputId() const noexcept { return m_output.ToOutputId(); }

    const Commitment& GetCommitment() const noexcept { return m_output.GetCommitment(); }
    const RangeProof::CPtr& GetRangeProof() const noexcept { return m_output.GetRangeProof(); }
    ProofData BuildProofData() const noexcept { return m_output.BuildProofData(); }

    IMPL_SERIALIZABLE(UTXO);
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(m_blockHeight);
        READWRITE(m_leafIdx);
        READWRITE(m_output);
    }

private:
    int32_t m_blockHeight;
    mmr::LeafIndex m_leafIdx;
    Output m_output;
};