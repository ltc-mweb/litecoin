#include <mw/models/tx/Kernel.h>
#include <mw/models/tx/KernelBits.h>

Serializer& Kernel::Serialize(Serializer& serializer) const noexcept
{
    uint8_t features_byte =
        (m_fee ? FEE_FEATURE_BIT : 0) |
        (m_pegin ? PEGIN_FEATURE_BIT : 0) |
        (m_pegout ? PEGOUT_FEATURE_BIT : 0) |
        (m_lockHeight ? HEIGHT_LOCK_FEATURE_BIT : 0) |
        (m_extraData.size() > 0 ? EXTRA_DATA_FEATURE_BIT : 0);

    serializer.Append<uint8_t>(features_byte);

    if (m_fee) {
        serializer.Append<uint64_t>(m_fee.value());
    }

    if (m_pegin) {
        serializer.Append<uint64_t>(m_pegin.value());
    }

    if (m_pegout) {
        serializer
            .Append<uint64_t>(m_pegout.value().GetAmount())
            .Append<uint8_t>((uint8_t)m_pegout.value().GetScriptPubKey().size())
            .Append(m_pegout.value().GetScriptPubKey());
    }

    if (m_lockHeight) {
        serializer.Append<uint64_t>(m_lockHeight.value());
    }

    if (!m_extraData.empty()) {
        serializer
            .Append<uint8_t>((uint8_t)m_extraData.size())
            .Append(m_extraData);
    }

    serializer
        .Append(m_excess)
        .Append(m_signature);

    return serializer;
}

Kernel Kernel::Deserialize(Deserializer& deserializer)
{
    uint8_t features = deserializer.Read<uint8_t>();

    // Workaround for gcc warning when using: boost::optional<uint64_t> fee = boost::none;
    auto fee([]() -> boost::optional<uint64_t> { return boost::none; }());
    if (features & FEE_FEATURE_BIT) {
        fee = deserializer.Read<uint64_t>();
    }

    // Workaround for gcc warning when using: boost::optional<uint64_t> pegin = boost::none;
    auto pegin([]() -> boost::optional<uint64_t> { return boost::none; }());
    if (features & PEGIN_FEATURE_BIT) {
        pegin = deserializer.Read<uint64_t>();
    }

    boost::optional<PegOutCoin> pegout = boost::none;
    if (features & PEGOUT_FEATURE_BIT) {
        uint64_t amount = deserializer.Read<uint64_t>();
        uint8_t num_bytes = deserializer.Read<uint8_t>();
        std::vector<uint8_t> scriptPubKey = deserializer.ReadVector(num_bytes);
        pegout = PegOutCoin(amount, std::move(scriptPubKey));
    }

    // Workaround for gcc warning when using: boost::optional<uint64_t> lock_height = boost::none;
    auto lock_height([]() -> boost::optional<uint64_t> { return boost::none; }());
    if (features & HEIGHT_LOCK_FEATURE_BIT) {
        lock_height = deserializer.Read<uint64_t>();
    }

    std::vector<uint8_t> extra_data;
    if (features & EXTRA_DATA_FEATURE_BIT) {
        uint8_t num_bytes = deserializer.Read<uint8_t>();
        extra_data = deserializer.ReadVector(num_bytes);
    }

    Commitment excess = Commitment::Deserialize(deserializer);
    Signature signature = Signature::Deserialize(deserializer);

    return Kernel{
        std::move(fee),
        std::move(pegin),
        std::move(pegout),
        std::move(lock_height),
        std::move(extra_data),
        std::move(excess),
        std::move(signature)
    };
}