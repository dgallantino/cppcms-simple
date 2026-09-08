#ifndef __PERSON_SERVICE_H__
#define __PERSON_SERVICE_H__

#include <data/PersonRepo.h>
#include <models/Person.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace cppdb {
    class session;
}

class DuplicateEmailError : public std::runtime_error
{
public:
    DuplicateEmailError() : std::runtime_error("personEmail already exists") {}
};

class PersonService
{
public:
    explicit PersonService(cppdb::session &sql);

    models::Person create(const models::Person &person);
    bool getById(int personId, models::Person &out);
    std::vector<models::Person> getAll();
    bool update(const models::Person &person);
    bool remove(int personId);

private:
    database::PersonRepo repo_;
    static bool isUniqueEmailError(const std::exception &e);
};

#endif
