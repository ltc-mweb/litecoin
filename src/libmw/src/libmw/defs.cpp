#include <libmw/defs.h>

#include <mw/node/INode.h>

LIBMW_NAMESPACE

libmw::CoinsViewRef CoinsViewRef::CreateCache() const
{
    if (pCoinsView == nullptr) {
        return libmw::CoinsViewRef{ nullptr };
    }

    return libmw::CoinsViewRef{ std::make_shared<mw::CoinsViewCache>(pCoinsView) };
}

END_NAMESPACE