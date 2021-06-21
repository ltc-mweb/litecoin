#pragma once

#include <boost/test/unit_test.hpp>
#include <mweb/mweb_db.h>
#include <test/test_bitcoin.h>

struct MWEBTestingSetup : public BasicTestingSetup {
    explicit MWEBTestingSetup()
        : BasicTestingSetup(CBaseChainParams::MAIN)
    {
        boost::filesystem::path datadir = SetDataDir(".mweb");
        ClearDatadirCache();
        m_db = std::make_unique<CDBWrapper>(datadir / "db", 1 << 15);
    }

    virtual ~MWEBTestingSetup() = default;

    mw::DBWrapper::Ptr GetDB() { return std::make_shared<MWEB::DBWrapper>(m_db.get()); }

private:
    std::unique_ptr<CDBWrapper> m_db;
};