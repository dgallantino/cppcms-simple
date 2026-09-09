#ifndef __AUTH_H__
#define __AUTH_H__

#include <cppcms/application.h>
#include <cppcms/service.h>
#include <controllers/Master.h>
#include <services/AuthService.h>
#include <string>

namespace cppdb {
    class session;
}

using database::Master;
class Auth : public Master
{
public:
    Auth(cppcms::service &srv);
    Auth(cppcms::service &srv, cppdb::session &sql);

    void login();
    void logout();

private:
    void mapUrls();
    void writeError(int status, const std::string &message);

    AuthService authService_;
};

#endif
