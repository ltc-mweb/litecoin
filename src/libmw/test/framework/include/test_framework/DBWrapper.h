#pragma once

#include <mw/interfaces/db_interface.h>
#include <mw/file/FilePath.h>

class TestDBWrapper
{
public:
    TestDBWrapper(const FilePath& db_path)
        : m_wrapper(boost::filesystem::path(db_path.u8string()), 1 << 15)
    {
        
    }

    mw::DBWrapper::Ptr Get() { return std::make_shared<mw::DBWrapper>(&m_wrapper); }

private:
    CDBWrapper m_wrapper;
};