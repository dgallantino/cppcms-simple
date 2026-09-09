#ifndef __MASTER_H__
#define __MASTER_H__

#include <cppcms/application.h>
#include <memory>
#include <string>

namespace cppdb {
    class session;
}

namespace database {
    class Master: public cppcms::application
    {
    public:
        Master(cppcms::service& srv);
        Master(cppcms::service& srv, cppdb::session &sql);
        ~Master();
    protected:
        cppdb::session &sql();
    private:
        std::auto_ptr<cppdb::session> sql_;
        cppdb::session *sql_external_;
        std::string conn_str_;
    };
}
#endif
