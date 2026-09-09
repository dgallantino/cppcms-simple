#include "UserService.h"
#include <ctime>

UserService::UserService(cppdb::session &sql) : repo_(sql)
{
}

std::string UserService::nowTimestamp()
{
    char buf[32];
    std::time_t t = std::time(0);
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return std::string(buf);
}

models::User UserService::create(const models::User &user)
{
    models::User existing;
    if (repo_.findByLoginId(user.usrsLoginId(), existing))
        throw DuplicateLoginIdError();

    models::User toSave = user;
    std::string now = nowTimestamp();
    toSave.setUsrsCreatedTime(now);
    toSave.setUsrsUpdatedTime(now);
    return repo_.insert(toSave);
}

bool UserService::getById(int usrsId, models::User &out)
{
    return repo_.findById(usrsId, out);
}

std::vector<models::User> UserService::getAll()
{
    return repo_.findAll();
}

bool UserService::update(const models::User &user)
{
    models::User existing;
    if (!repo_.findById(user.usrsId(), existing))
        return false;

    models::User other;
    if (repo_.findByLoginId(user.usrsLoginId(), other) && other.usrsId() != user.usrsId())
        throw DuplicateLoginIdError();

    models::User toSave = user;
    toSave.setUsrsCreatedTime(existing.usrsCreatedTime());
    toSave.setUsrsUpdatedTime(nowTimestamp());
    return repo_.update(toSave);
}

bool UserService::remove(int usrsId)
{
    return repo_.remove(usrsId);
}
