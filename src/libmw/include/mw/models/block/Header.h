#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Macros.h>
#include <mw/common/Traits.h>
#include <mw/crypto/Hasher.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/crypto/Hash.h>

#include <cstdint>
#include <memory>

MW_NAMESPACE

class Header final :
    public Traits::IPrintable,
    public Traits::ISerializable,
    public Traits::IHashable
{
public:
    using CPtr = std::shared_ptr<const Header>;

    //
    // Constructors
    //
    Header() = default;
    Header(
        const int32_t height,
        mw::Hash&& outputRoot,
        mw::Hash&& kernelRoot,
		mw::Hash&& leafsetRoot,
        BlindingFactor&& kernelOffset,
        BlindingFactor&& ownerOffset,
        const uint64_t outputMMRSize,
        const uint64_t kernelMMRSize
    )
        : m_height(height),
        m_outputRoot(std::move(outputRoot)),
        m_kernelRoot(std::move(kernelRoot)),
		m_leafsetRoot(std::move(leafsetRoot)),
        m_kernelOffset(std::move(kernelOffset)),
        m_ownerOffset(std::move(ownerOffset)),
        m_outputMMRSize(outputMMRSize),
        m_kernelMMRSize(kernelMMRSize)
    {
        m_hash = Hashed(*this);
    }

    //
    // Operators
    //
    bool operator==(const Header& rhs) const noexcept { return this->GetHash() == rhs.GetHash(); }
    bool operator!=(const Header& rhs) const noexcept { return this->GetHash() != rhs.GetHash(); }

    //
    // Getters
    //
    int32_t GetHeight() const noexcept { return m_height; }
    const mw::Hash& GetOutputRoot() const noexcept { return m_outputRoot; }
    const mw::Hash& GetKernelRoot() const noexcept { return m_kernelRoot; }
	const mw::Hash& GetLeafsetRoot() const noexcept { return m_leafsetRoot; }
    const BlindingFactor& GetKernelOffset() const noexcept { return m_kernelOffset; }
    const BlindingFactor& GetOwnerOffset() const noexcept { return m_ownerOffset; }
    uint64_t GetNumTXOs() const noexcept { return m_outputMMRSize; }
    uint64_t GetNumKernels() const noexcept { return m_kernelMMRSize; }

    //
    // Traits
    //
    const mw::Hash& GetHash() const noexcept final { return m_hash; }

    std::string Format() const final { return GetHash().ToHex(); }

    //
    // Serialization/Deserialization
    //
    IMPL_SERIALIZABLE(Header);
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(VARINT(m_height, VarIntMode::NONNEGATIVE_SIGNED));
        READWRITE(m_outputRoot);
        READWRITE(m_kernelRoot);
        READWRITE(m_leafsetRoot);
        READWRITE(m_kernelOffset);
        READWRITE(m_ownerOffset);
        READWRITE(VARINT(m_outputMMRSize));
        READWRITE(VARINT(m_kernelMMRSize));

        if (ser_action.ForRead()) {
            m_hash = Hashed(*this);
        }
    }

private:
    int32_t m_height;
    mw::Hash m_outputRoot;
    mw::Hash m_kernelRoot;
	mw::Hash m_leafsetRoot;
    BlindingFactor m_kernelOffset;
    BlindingFactor m_ownerOffset;
    uint64_t m_outputMMRSize;
    uint64_t m_kernelMMRSize;

    mw::Hash m_hash;
};

END_NAMESPACE