#ifndef __PERSON_REPO_H__
#define __PERSON_REPO_H__

#include <models/Person.h>
#include <vector>

namespace cppdb {
    class session;
    class result;
}

namespace database {

class PersonRepo
{
public:
    explicit PersonRepo(cppdb::session &sql);

    models::Person insert(const models::Person &person);
    bool findById(int personId, models::Person &out);
    std::vector<models::Person> findAll();
    bool update(const models::Person &person);
    bool remove(int personId);

private:
    cppdb::session &sql_;
    models::Person fromRow(cppdb::result &res);
};

}

#endif
