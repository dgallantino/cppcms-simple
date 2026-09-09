#ifndef __DB_CONNECTION__
#define __DB_CONNECTION__

#include <cppdb/frontend.h>

namespace database {
    class Connection {
    protected:
        Connection(const std::string& constr) : sql(constr) {}
        static Connection* connection_;
    public:
        cppdb::session sql;
        static Connection *getInstance(const std::string& constr);
    };
}

#endif