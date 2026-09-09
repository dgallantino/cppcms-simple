#ifndef __PERSON_H__
#define __PERSON_H__

#include <cppcms/application.h>
#include <cppcms/service.h>
#include <controllers/Master.h>
#include <models/Person.h>
#include <services/PersonService.h>
#include <string>
#include <vector>

namespace cppdb {
    class session;
}

using database::Master;

class Person : public Master
{
public:
    Person(cppcms::service &srv);
    Person(cppcms::service &srv, cppdb::session &sql);

    void collection();
    void item(std::string id);

private:
    void mapUrls();
    bool requireAuth();
    bool parsePersonBody(models::Person &out, std::string &error);
    void writePerson(int status, const models::Person &person);
    void writePersonList(const std::vector<models::Person> &people);
    void writeError(int status, const std::string &message);

    PersonService personService_;
};

#endif
