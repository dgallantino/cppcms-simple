#include "User.h"
#include <cppcms/service.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/http_response.h>
#include <cppcms/http_request.h>
#include <picojson.h>
#include <cstdlib>

User::User(cppcms::service &srv)
    : Master(srv), authService_(sql()), userService_(sql())
{
    mapUrls();
}

User::User(cppcms::service &srv, cppdb::session &sql)
    : Master(srv, sql), authService_(sql), userService_(sql)
{
    mapUrls();
}

void User::mapUrls()
{
    dispatcher().assign("", &User::collection, this);
    mapper().assign("");

    dispatcher().assign("/(\\d+)", &User::item, this, 1);
    mapper().assign("/{1}");
}

bool User::requireAuth()
{
    std::string token = request().getenv("HTTP_TOKEN");
    if (authService_.isValidToken(token))
        return true;
    writeError(cppcms::http::response::unauthorized, "Unauthorized");
    return false;
}

bool User::parseUserBody(models::User &out, std::string &error)
{
    std::pair<void *, size_t> post_data = request().raw_post_data();
    if (post_data.second == 0) {
        error = "JSON body is required";
        return false;
    }

    std::string rawData(reinterpret_cast<char const *>(post_data.first), post_data.second);
    picojson::value root;
    std::string parseError;
    picojson::parse(root, rawData.c_str(), rawData.c_str() + rawData.size(), &parseError);
    if (!parseError.empty() || !root.is<picojson::object>()) {
        error = "JSON Invalid";
        return false;
    }

    picojson::object &obj = root.get<picojson::object>();

    picojson::object::iterator loginIt = obj.find("usrsLoginId");
    if (loginIt == obj.end() || !loginIt->second.is<std::string>() || loginIt->second.get<std::string>().empty()) {
        error = "usrsLoginId is required";
        return false;
    }

    picojson::object::iterator passIt = obj.find("usrsLoginPass");
    if (passIt == obj.end() || !passIt->second.is<std::string>() || passIt->second.get<std::string>().empty()) {
        error = "usrsLoginPass is required";
        return false;
    }

    out.setUsrsLoginId(loginIt->second.get<std::string>());
    out.setUsrsLoginPass(passIt->second.get<std::string>());

    picojson::object::iterator firstIt = obj.find("usrsFirstName");
    if (firstIt == obj.end() || firstIt->second.is<picojson::null>()) {
        out.setUsrsFirstName("");
    } else if (firstIt->second.is<std::string>()) {
        out.setUsrsFirstName(firstIt->second.get<std::string>());
    } else {
        error = "usrsFirstName must be a string";
        return false;
    }

    picojson::object::iterator lastIt = obj.find("usrsLastName");
    if (lastIt == obj.end() || lastIt->second.is<picojson::null>()) {
        out.setUsrsLastName("");
    } else if (lastIt->second.is<std::string>()) {
        out.setUsrsLastName(lastIt->second.get<std::string>());
    } else {
        error = "usrsLastName must be a string";
        return false;
    }

    return true;
}

static picojson::value userToJson(const models::User &user)
{
    picojson::object obj;
    obj["usrsId"] = picojson::value(static_cast<double>(user.usrsId()));
    obj["usrsFirstName"] = picojson::value(user.usrsFirstName());
    obj["usrsLastName"] = picojson::value(user.usrsLastName());
    obj["usrsLoginId"] = picojson::value(user.usrsLoginId());
    obj["usrsLoginPass"] = picojson::value(user.usrsLoginPass());
    obj["usrsCreatedTime"] = picojson::value(user.usrsCreatedTime());
    obj["usrsUpdatedTime"] = picojson::value(user.usrsUpdatedTime());
    return picojson::value(obj);
}

void User::writeUser(int status, const models::User &user)
{
    response().status(status);
    response().content_type("application/json");
    response().out() << userToJson(user).serialize();
}

void User::writeUserList(const std::vector<models::User> &users)
{
    picojson::array arr;
    for (std::vector<models::User>::const_iterator it = users.begin(); it != users.end(); ++it)
        arr.push_back(userToJson(*it));
    response().status(cppcms::http::response::ok);
    response().content_type("application/json");
    response().out() << picojson::value(arr).serialize();
}

void User::writeError(int status, const std::string &message)
{
    picojson::object obj;
    obj["error"] = picojson::value(message);
    response().status(status);
    response().content_type("application/json");
    response().out() << picojson::value(obj).serialize();
}

void User::collection()
{
    if (!requireAuth())
        return;

    std::string method = request().request_method();
    try {
        if (method == "GET") {
            writeUserList(userService_.getAll());
            return;
        }
        if (method == "POST") {
            models::User user;
            std::string error;
            if (!parseUserBody(user, error)) {
                writeError(cppcms::http::response::bad_request, error);
                return;
            }
            models::User created = userService_.create(user);
            writeUser(cppcms::http::response::created, created);
            return;
        }
        writeError(cppcms::http::response::method_not_allowed, "Method not allowed");
    } catch (const DuplicateLoginIdError &e) {
        writeError(cppcms::http::response::conflict, e.what());
    } catch (const std::exception &e) {
        writeError(cppcms::http::response::internal_server_error, e.what());
    }
}

void User::item(std::string id)
{
    if (!requireAuth())
        return;

    int usrsId = std::atoi(id.c_str());
    std::string method = request().request_method();
    try {
        if (method == "GET") {
            models::User user;
            if (!userService_.getById(usrsId, user)) {
                writeError(cppcms::http::response::not_found, "User not found");
                return;
            }
            writeUser(cppcms::http::response::ok, user);
            return;
        }
        if (method == "PUT") {
            models::User user;
            std::string error;
            if (!parseUserBody(user, error)) {
                writeError(cppcms::http::response::bad_request, error);
                return;
            }
            user.setUsrsId(usrsId);
            if (!userService_.update(user)) {
                writeError(cppcms::http::response::not_found, "User not found");
                return;
            }
            userService_.getById(usrsId, user);
            writeUser(cppcms::http::response::ok, user);
            return;
        }
        if (method == "DELETE") {
            if (!userService_.remove(usrsId)) {
                writeError(cppcms::http::response::not_found, "User not found");
                return;
            }
            picojson::object obj;
            obj["ok"] = picojson::value(true);
            response().status(cppcms::http::response::ok);
            response().content_type("application/json");
            response().out() << picojson::value(obj).serialize();
            return;
        }
        writeError(cppcms::http::response::method_not_allowed, "Method not allowed");
    } catch (const DuplicateLoginIdError &e) {
        writeError(cppcms::http::response::conflict, e.what());
    } catch (const std::exception &e) {
        writeError(cppcms::http::response::internal_server_error, e.what());
    }
}
