#include "Auth.h"
#include <cppcms/service.h>  
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/http_response.h>
#include <cppdb/frontend.h>
#include <booster/locale/conversion.h>
#include <cppcms/http_request.h>
#include <picojson.h>

// constructor
Auth::Auth(cppcms::service &srv) : Master(srv)
{
    dispatcher().assign("/login", &Auth::login, this);
    mapper().assign("/login");

    dispatcher().assign("/logout", &Auth::logout, this);
    mapper().assign("/logout");
}

// method login
void Auth::login()
{
    std::string req_method = request().request_method();
    if (req_method.compare("POST") != 0) {
        response().out() << "Method does not allowed";
        return;
    }
    std::pair<void *,size_t> post_data = request().raw_post_data();
    std::string rawData = std::string(reinterpret_cast<char const *>(post_data.first),post_data.second);
    picojson::value _user;
    const char* jsonUser = rawData.c_str();
    std::string is_error;
    const char* json_end = picojson::parse(_user, jsonUser, jsonUser + strlen(jsonUser), &is_error);
    if (!is_error.empty()) {
        response().out() << "JSON Invalid";
        return;
    }
    std::string LoginID = _user.get("User").get("LoginId").get<std::string>();
    std::string Password = _user.get("User").get("Password").get<std::string>();
    cppdb::result res = sql() << "SELECT * FROM users WHERE usrsLoginId = ?" << LoginID;

    if(res.next()) {
        int colFirstName = res.find_column("usrsFirstName");
        std::string firstName;
        res.fetch(colFirstName, firstName);
        int colPassword = res.find_column("usrsLoginPass");
        std::string _u_password;
        res.fetch(colPassword, _u_password);
        
        if (_u_password == Password) {
            // TODO: add jwt token
            response().out() << "User accepted";
            return;
        }
    }
    response().out() <<"Login invalid";
}

// method logout
void Auth::logout()
{
    response().out() <<"Logout API ";
}
