#include "UserRepo.h"
#include <cppdb/frontend.h>

namespace database {

static std::string fetchText(cppdb::result &res, const char *column)
{
    int col = res.find_column(column);
    if (res.is_null(col))
        return std::string();
    std::string value;
    res.fetch(col, value);
    return value;
}

UserRepo::UserRepo(cppdb::session &sql) : sql_(sql)
{
}

models::User UserRepo::fromRow(cppdb::result &res)
{
    models::User user;

    int usrsId = 0;
    res.fetch(res.find_column("usrsId"), usrsId);
    user.setUsrsId(usrsId);
    user.setUsrsFirstName(fetchText(res, "usrsFirstName"));
    user.setUsrsLastName(fetchText(res, "usrsLastName"));
    user.setUsrsLoginId(fetchText(res, "usrsLoginId"));
    user.setUsrsLoginPass(fetchText(res, "usrsLoginPass"));
    user.setUsrsCreatedTime(fetchText(res, "usrsCreatedTime"));
    user.setUsrsUpdatedTime(fetchText(res, "usrsUpdatedTime"));
    return user;
}

models::User UserRepo::insert(const models::User &user)
{
    cppdb::statement st = sql_
        << "INSERT INTO users (usrsFirstName, usrsLastName, usrsLoginId, usrsLoginPass, "
           "usrsCreatedTime, usrsUpdatedTime) VALUES (?, ?, ?, ?, ?, ?)"
        << user.usrsFirstName()
        << user.usrsLastName()
        << user.usrsLoginId()
        << user.usrsLoginPass()
        << user.usrsCreatedTime()
        << user.usrsUpdatedTime()
        << cppdb::exec;

    models::User created = user;
    created.setUsrsId(static_cast<int>(st.last_insert_id()));
    return created;
}

bool UserRepo::findById(int usrsId, models::User &out)
{
    cppdb::result res = sql_
        << "SELECT usrsId, usrsFirstName, usrsLastName, usrsLoginId, usrsLoginPass, "
           "usrsCreatedTime, usrsUpdatedTime FROM users WHERE usrsId = ?"
        << usrsId;
    if (!res.next())
        return false;
    out = fromRow(res);
    return true;
}

bool UserRepo::findByLoginId(const std::string &loginId, models::User &out)
{
    cppdb::result res = sql_
        << "SELECT usrsId, usrsFirstName, usrsLastName, usrsLoginId, usrsLoginPass, "
           "usrsCreatedTime, usrsUpdatedTime FROM users WHERE usrsLoginId = ?"
        << loginId;
    if (!res.next())
        return false;
    out = fromRow(res);
    return true;
}

std::vector<models::User> UserRepo::findAll()
{
    std::vector<models::User> users;
    cppdb::result res = sql_
        << "SELECT usrsId, usrsFirstName, usrsLastName, usrsLoginId, usrsLoginPass, "
           "usrsCreatedTime, usrsUpdatedTime FROM users";
    while (res.next())
        users.push_back(fromRow(res));
    return users;
}

bool UserRepo::update(const models::User &user)
{
    cppdb::statement st = sql_
        << "UPDATE users SET usrsFirstName = ?, usrsLastName = ?, usrsLoginId = ?, "
           "usrsLoginPass = ?, usrsCreatedTime = ?, usrsUpdatedTime = ? WHERE usrsId = ?"
        << user.usrsFirstName()
        << user.usrsLastName()
        << user.usrsLoginId()
        << user.usrsLoginPass()
        << user.usrsCreatedTime()
        << user.usrsUpdatedTime()
        << user.usrsId()
        << cppdb::exec;
    return st.affected() > 0;
}

bool UserRepo::remove(int usrsId)
{
    cppdb::statement st = sql_
        << "DELETE FROM users WHERE usrsId = ?"
        << usrsId << cppdb::exec;
    return st.affected() > 0;
}

}
