#ifndef __USER_H__
#define __USER_H__

#include <cppcms/application.h>
#include <cppcms/service.h>
#include <controllers/Master.h>

using database::Master;

class User : public Master
{
public:
    User(cppcms::service &srv);

    void getUser();
    void getUserById(std::string id);

};

#endif