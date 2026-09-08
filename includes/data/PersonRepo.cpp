#include "PersonRepo.h"
#include <cppdb/frontend.h>

namespace database {

PersonRepo::PersonRepo(cppdb::session &sql) : sql_(sql)
{
}

models::Person PersonRepo::fromRow(cppdb::result &res)
{
    models::Person person;

    int personId = 0;
    res.fetch(res.find_column("personId"), personId);
    person.setPersonId(personId);

    std::string personName;
    res.fetch(res.find_column("personName"), personName);
    person.setPersonName(personName);

    std::string personEmail;
    res.fetch(res.find_column("personEmail"), personEmail);
    person.setPersonEmail(personEmail);

    int addrCol = res.find_column("personAddress");
    if (res.is_null(addrCol)) {
        person.setPersonAddressNull();
    } else {
        std::string personAddress;
        res.fetch(addrCol, personAddress);
        person.setPersonAddress(personAddress);
    }

    return person;
}

models::Person PersonRepo::insert(const models::Person &person)
{
    cppdb::statement st = sql_
        << "INSERT INTO person (personName, personEmail, personAddress) VALUES (?, ?, ?)";
    st << person.personName() << person.personEmail();
    if (person.personAddressIsNull())
        st << cppdb::null;
    else
        st << person.personAddress();
    st << cppdb::exec;

    models::Person created = person;
    created.setPersonId(static_cast<int>(st.last_insert_id()));
    return created;
}

bool PersonRepo::findById(int personId, models::Person &out)
{
    cppdb::result res = sql_
        << "SELECT personId, personName, personEmail, personAddress FROM person WHERE personId = ?"
        << personId;
    if (!res.next())
        return false;
    out = fromRow(res);
    return true;
}

std::vector<models::Person> PersonRepo::findAll()
{
    std::vector<models::Person> people;
    cppdb::result res = sql_
        << "SELECT personId, personName, personEmail, personAddress FROM person";
    while (res.next())
        people.push_back(fromRow(res));
    return people;
}

bool PersonRepo::update(const models::Person &person)
{
    cppdb::statement st = sql_
        << "UPDATE person SET personName = ?, personEmail = ?, personAddress = ? WHERE personId = ?";
    st << person.personName() << person.personEmail();
    if (person.personAddressIsNull())
        st << cppdb::null;
    else
        st << person.personAddress();
    st << person.personId() << cppdb::exec;
    return st.affected() > 0;
}

bool PersonRepo::remove(int personId)
{
    cppdb::statement st = sql_
        << "DELETE FROM person WHERE personId = ?"
        << personId << cppdb::exec;
    return st.affected() > 0;
}

}
