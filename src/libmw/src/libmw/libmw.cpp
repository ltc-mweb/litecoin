#include <libmw/libmw.h>

#include <mw/node/State.h>

LIBMW_NAMESPACE

libmw::StateRef DeserializeState(const std::vector<uint8_t>& bytes)
{
    Deserializer deserializer{ bytes };
    mw::State state = mw::State::Deserialize(deserializer);
    return { std::make_shared<mw::State>(std::move(state)) };
}

std::vector<uint8_t> SerializeState(const libmw::StateRef& state)
{
    assert(state.pState != nullptr);
    return state.pState->Serialized();
}

END_NAMESPACE // libmw