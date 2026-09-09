#ifndef __USER_SERVICE_H__
#define __USER_SERVICE_H__

#include <data/UserRepo.h>
#include <models/User.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace cppdb {
    class session;
}

class DuplicateLoginIdError : public std::runtime_error
{
public:
    DuplicateLoginIdError() : std::runtime_error("usrsLoginId already exists") {}
};

class UserService
{
public:
    explicit UserService(cppdb::session &sql);

    models::User create(const models::User &user);
    bool getById(int usrsId, models::User &out);
    std::vector<models::User> getAll();
    bool update(const models::User &user);
    bool remove(int usrsId);

private:
    database::UserRepo repo_;
    static std::string nowTimestamp();
};

#endif
