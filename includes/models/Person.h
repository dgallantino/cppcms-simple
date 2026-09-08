#ifndef __MODELS_PERSON_H__
#define __MODELS_PERSON_H__

#include <string>

namespace models {

class Person
{
public:
    Person() : personId_(0), personAddressIsNull_(true) {}

    int personId() const { return personId_; }
    void setPersonId(int id) { personId_ = id; }

    const std::string &personName() const { return personName_; }
    void setPersonName(const std::string &name) { personName_ = name; }

    const std::string &personEmail() const { return personEmail_; }
    void setPersonEmail(const std::string &email) { personEmail_ = email; }

    const std::string &personAddress() const { return personAddress_; }
    bool personAddressIsNull() const { return personAddressIsNull_; }
    void setPersonAddress(const std::string &address)
    {
        personAddress_ = address;
        personAddressIsNull_ = false;
    }
    void setPersonAddressNull()
    {
        personAddress_.clear();
        personAddressIsNull_ = true;
    }

private:
    int personId_;
    std::string personName_;
    std::string personEmail_;
    std::string personAddress_;
    bool personAddressIsNull_;
};

}

#endif
