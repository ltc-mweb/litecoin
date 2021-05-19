#include <mw/models/tx/TxBody.h>

Serializer& TxBody::Serialize(Serializer& serializer) const noexcept
{
    return serializer
        .AppendVec<Input>(m_inputs)
        .AppendVec<Output>(m_outputs)
        .AppendVec<Kernel>(m_kernels)
        .AppendVec<SignedMessage>(m_ownerSigs);
}

TxBody TxBody::Deserialize(Deserializer& deserializer)
{
    std::vector<Input> inputs = deserializer.ReadVec<Input>();
    std::vector<Output> outputs = deserializer.ReadVec<Output>();
    std::vector<Kernel> kernels = deserializer.ReadVec<Kernel>();
    std::vector<SignedMessage> owner_sigs = deserializer.ReadVec<SignedMessage>();
    return TxBody(std::move(inputs), std::move(outputs), std::move(kernels), std::move(owner_sigs));
}