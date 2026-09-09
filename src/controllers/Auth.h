#ifndef __AUTH_H__
#define __AUTH_H__

#include <cppcms/application.h>
#include <cppcms/service.h>
#include <controllers/Master.h>

using database::Master;
class Auth : public Master
{
public:
    Auth(cppcms::service &srv);

    void login();
    void logout();

};

#endif