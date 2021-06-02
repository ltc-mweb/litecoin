#include <mw/traits/Serializable.h>

class SerializableVec : public Traits::ISerializable
{
public:
    SerializableVec() = default;
    SerializableVec(std::vector<uint8_t> bytes) : m_bytes(std::move(bytes)) {}

    const std::vector<uint8_t>& Get() const { return m_bytes; }

    IMPL_SERIALIZABLE;

    template <typename Stream>
    void Serialize(Stream& s) const
    {
        s.write((const char*)m_bytes.data(), m_bytes.size());
    }

    template <typename Stream>
    void Unserialize(Stream& s)
    {
        s.read((char*)m_bytes.data(), s.size());
    }

private:
    std::vector<uint8_t> m_bytes;
};