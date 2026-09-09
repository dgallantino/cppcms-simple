#include "Auth.h"
#include <cppcms/service.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/http_response.h>
#include <cppcms/http_request.h>
#include <picojson.h>

Auth::Auth(cppcms::service &srv) : Master(srv), authService_(sql())
{
    mapUrls();
}

Auth::Auth(cppcms::service &srv, cppdb::session &sql)
    : Master(srv, sql), authService_(sql)
{
    mapUrls();
}

void Auth::mapUrls()
{
    dispatcher().assign("/login", &Auth::login, this);
    mapper().assign("/login");

    dispatcher().assign("/logout", &Auth::logout, this);
    mapper().assign("/logout");
}

void Auth::writeError(int status, const std::string &message)
{
    picojson::object obj;
    obj["error"] = picojson::value(message);
    response().status(status);
    response().content_type("application/json");
    response().out() << picojson::value(obj).serialize();
}

static void writeJson(cppcms::http::response &response, int status, const picojson::object &obj)
{
    response.status(status);
    response.content_type("application/json");
    response.out() << picojson::value(obj).serialize();
}

void Auth::login()
{
    if (request().request_method() != "POST") {
        writeError(cppcms::http::response::method_not_allowed, "Method not allowed");
        return;
    }

    std::pair<void *, size_t> post_data = request().raw_post_data();
    if (post_data.second == 0) {
        writeError(cppcms::http::response::bad_request, "JSON body is required");
        return;
    }

    std::string rawData(reinterpret_cast<char const *>(post_data.first), post_data.second);
    picojson::value root;
    std::string parseError;
    picojson::parse(root, rawData.c_str(), rawData.c_str() + rawData.size(), &parseError);
    if (!parseError.empty() || !root.is<picojson::object>()) {
        writeError(cppcms::http::response::bad_request, "JSON Invalid");
        return;
    }

    picojson::object &obj = root.get<picojson::object>();
    picojson::object::iterator userIt = obj.find("User");
    if (userIt == obj.end() || !userIt->second.is<picojson::object>()) {
        writeError(cppcms::http::response::bad_request, "User is required");
        return;
    }

    picojson::object &userObj = userIt->second.get<picojson::object>();
    picojson::object::iterator loginIt = userObj.find("LoginId");
    picojson::object::iterator passIt = userObj.find("Password");
    if (loginIt == userObj.end() || !loginIt->second.is<std::string>() || loginIt->second.get<std::string>().empty()) {
        writeError(cppcms::http::response::bad_request, "LoginId is required");
        return;
    }
    if (passIt == userObj.end() || !passIt->second.is<std::string>() || passIt->second.get<std::string>().empty()) {
        writeError(cppcms::http::response::bad_request, "Password is required");
        return;
    }

    try {
        std::string token = authService_.login(
            loginIt->second.get<std::string>(),
            passIt->second.get<std::string>());
        if (token.empty()) {
            writeError(cppcms::http::response::unauthorized, "Login invalid");
            return;
        }
        picojson::object ok;
        ok["token"] = picojson::value(token);
        writeJson(response(), cppcms::http::response::ok, ok);
    } catch (const std::exception &e) {
        writeError(cppcms::http::response::internal_server_error, e.what());
    }
}

void Auth::logout()
{
    if (request().request_method() != "POST") {
        writeError(cppcms::http::response::method_not_allowed, "Method not allowed");
        return;
    }

    std::string token = request().getenv("HTTP_TOKEN");
    if (!authService_.logout(token)) {
        writeError(cppcms::http::response::unauthorized, "Unauthorized");
        return;
    }

    picojson::object ok;
    ok["ok"] = picojson::value(true);
    writeJson(response(), cppcms::http::response::ok, ok);
}
