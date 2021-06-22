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
        m_mweb_db = std::make_shared<MWEB::DBWrapper>(m_db.get());
    }

    virtual ~MWEBTestingSetup() = default;

    mw::DBWrapper::Ptr GetDB() { return m_mweb_db; }

private:
    std::unique_ptr<CDBWrapper> m_db;
    std::shared_ptr<mw::DBWrapper> m_mweb_db;
};