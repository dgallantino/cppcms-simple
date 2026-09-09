#ifndef __USER_REPO_H__
#define __USER_REPO_H__

#include <models/User.h>
#include <string>
#include <vector>

namespace cppdb {
    class session;
    class result;
}

namespace database {

class UserRepo
{
public:
    explicit UserRepo(cppdb::session &sql);

    models::User insert(const models::User &user);
    bool findById(int usrsId, models::User &out);
    bool findByLoginId(const std::string &loginId, models::User &out);
    std::vector<models::User> findAll();
    bool update(const models::User &user);
    bool remove(int usrsId);

private:
    cppdb::session &sql_;
    models::User fromRow(cppdb::result &res);
};

}

#endif
