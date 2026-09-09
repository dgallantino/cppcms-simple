#ifndef __AUTH_SERVICE_H__
#define __AUTH_SERVICE_H__

#include <data/UserRepo.h>
#include <string>

namespace cppdb {
    class session;
}

class AuthService
{
public:
    explicit AuthService(cppdb::session &sql);

    std::string login(const std::string &loginId, const std::string &password);
    bool logout(const std::string &token);
    bool isValidToken(const std::string &token) const;

private:
    database::UserRepo repo_;
    static std::string generateToken();
};

#endif
