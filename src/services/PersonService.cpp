#include "PersonService.h"
#include <cppdb/errors.h>
#include <cstring>

PersonService::PersonService(cppdb::session &sql) : repo_(sql)
{
}

bool PersonService::isUniqueEmailError(const std::exception &e)
{
    const char *msg = e.what();
    if (msg == 0)
        return false;
    return std::strstr(msg, "UNIQUE") != 0 || std::strstr(msg, "unique") != 0;
}

models::Person PersonService::create(const models::Person &person)
{
    try {
        return repo_.insert(person);
    } catch (const cppdb::cppdb_error &e) {
        if (isUniqueEmailError(e))
            throw DuplicateEmailError();
        throw;
    }
}

bool PersonService::getById(int personId, models::Person &out)
{
    return repo_.findById(personId, out);
}

std::vector<models::Person> PersonService::getAll()
{
    return repo_.findAll();
}

bool PersonService::update(const models::Person &person)
{
    try {
        return repo_.update(person);
    } catch (const cppdb::cppdb_error &e) {
        if (isUniqueEmailError(e))
            throw DuplicateEmailError();
        throw;
    }
}

bool PersonService::remove(int personId)
{
    return repo_.remove(personId);
}
