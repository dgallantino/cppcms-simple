#include "Master.h"
#include <cppdb/frontend.h>
#include <cppcms/json.h>

namespace database {

Master::Master(cppcms::service &srv)
    : cppcms::application(srv), sql_external_(0)
{
}

Master::Master(cppcms::service &srv, cppdb::session &sql)
    : cppcms::application(srv), sql_external_(&sql)
{
}

Master::~Master()
{
}

cppdb::session &Master::sql()
{
    if (sql_external_)
        return *sql_external_;
    if (!sql_.get()) {
        conn_str_ = settings().get<std::string>("cppcms_simple.connection_string");
        sql_.reset(new cppdb::session(conn_str_));
    } else if (!sql_->is_open()) {
        sql_->open(conn_str_);
    }
    return *sql_;
}

}
