#ifndef __USER_H__
#define __USER_H__

#include <cppcms/application.h>
#include <cppcms/service.h>
#include <controllers/Master.h>
#include <models/User.h>
#include <services/AuthService.h>
#include <services/UserService.h>
#include <string>
#include <vector>

namespace cppdb {
    class session;
}

using database::Master;

class User : public Master
{
public:
    User(cppcms::service &srv);
    User(cppcms::service &srv, cppdb::session &sql);

    void collection();
    void item(std::string id);

private:
    void mapUrls();
    bool requireAuth();
    bool parseUserBody(models::User &out, std::string &error);
    void writeUser(int status, const models::User &user);
    void writeUserList(const std::vector<models::User> &users);
    void writeError(int status, const std::string &message);

    AuthService authService_;
    UserService userService_;
};

#endif
