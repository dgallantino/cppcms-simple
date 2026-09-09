#ifndef __MODELS_USER_H__
#define __MODELS_USER_H__

#include <string>

namespace models {

class User
{
public:
    User() : usrsId_(0) {}

    int usrsId() const { return usrsId_; }
    void setUsrsId(int id) { usrsId_ = id; }

    const std::string &usrsFirstName() const { return usrsFirstName_; }
    void setUsrsFirstName(const std::string &name) { usrsFirstName_ = name; }

    const std::string &usrsLastName() const { return usrsLastName_; }
    void setUsrsLastName(const std::string &name) { usrsLastName_ = name; }

    const std::string &usrsLoginId() const { return usrsLoginId_; }
    void setUsrsLoginId(const std::string &loginId) { usrsLoginId_ = loginId; }

    const std::string &usrsLoginPass() const { return usrsLoginPass_; }
    void setUsrsLoginPass(const std::string &password) { usrsLoginPass_ = password; }

    const std::string &usrsCreatedTime() const { return usrsCreatedTime_; }
    void setUsrsCreatedTime(const std::string &time) { usrsCreatedTime_ = time; }

    const std::string &usrsUpdatedTime() const { return usrsUpdatedTime_; }
    void setUsrsUpdatedTime(const std::string &time) { usrsUpdatedTime_ = time; }

private:
    int usrsId_;
    std::string usrsFirstName_;
    std::string usrsLastName_;
    std::string usrsLoginId_;
    std::string usrsLoginPass_;
    std::string usrsCreatedTime_;
    std::string usrsUpdatedTime_;
};

}

#endif
