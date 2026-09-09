#include "AuthService.h"
#include <helpers/TokenManager.h>
#include <iomanip>
#include <random>
#include <sstream>

AuthService::AuthService(cppdb::session &sql) : repo_(sql)
{
}

std::string AuthService::generateToken()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i)
        ss << std::setw(2) << dist(gen);
    return ss.str();
}

std::string AuthService::login(const std::string &loginId, const std::string &password)
{
    models::User user;
    if (!repo_.findByLoginId(loginId, user))
        return std::string();
    if (user.usrsLoginPass() != password)
        return std::string();

    std::string token = generateToken();
    std::ostringstream data;
    data << "{\"usrsId\":" << user.usrsId()
         << ",\"usrsLoginId\":\"" << user.usrsLoginId() << "\"}";
    TokenManager::getInstance().addToken(token, data.str());
    return token;
}

bool AuthService::logout(const std::string &token)
{
    if (!isValidToken(token))
        return false;
    TokenManager::getInstance().removeToken(token);
    return true;
}

bool AuthService::isValidToken(const std::string &token) const
{
    return TokenManager::getInstance().isValidToken(token);
}
