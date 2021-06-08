#include <mw/traits/Serializable.h>

class SerializableVec : public Traits::ISerializable
{
public:
    SerializableVec() = default;
    SerializableVec(std::vector<uint8_t> bytes) : m_bytes(std::move(bytes)) {}

    const std::vector<uint8_t>& Get() const { return m_bytes; }

    IMPL_SERIALIZABLE;
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(m_bytes);
    }

private:
    std::vector<uint8_t> m_bytes;
};