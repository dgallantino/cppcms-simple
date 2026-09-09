#include "Person.h"
#include <cppcms/service.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/http_response.h>
#include <cppcms/http_request.h>
#include <picojson.h>
#include <cstdlib>
#include <helpers/TokenManager.h>

Person::Person(cppcms::service &srv) : Master(srv), personService_(sql())
{
    mapUrls();
}

Person::Person(cppcms::service &srv, cppdb::session &sql)
    : Master(srv, sql), personService_(sql)
{
    mapUrls();
}

void Person::mapUrls()
{
    dispatcher().assign("", &Person::collection, this);
    mapper().assign("");

    dispatcher().assign("/(\\d+)", &Person::item, this, 1);
    mapper().assign("/{1}");
}

bool Person::requireAuth()
{
    TokenManager& tokenManager = TokenManager::getInstance();
    std::string token = request().getenv("HTTP_TOKEN");
    if (tokenManager.isValidToken(token))
        return true;
    writeError(cppcms::http::response::unauthorized, "Unauthorized");
    return false;
}

bool Person::parsePersonBody(models::Person &out, std::string &error)
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

    picojson::object::iterator nameIt = obj.find("personName");
    if (nameIt == obj.end() || !nameIt->second.is<std::string>() || nameIt->second.get<std::string>().empty()) {
        error = "personName is required";
        return false;
    }

    picojson::object::iterator emailIt = obj.find("personEmail");
    if (emailIt == obj.end() || !emailIt->second.is<std::string>() || emailIt->second.get<std::string>().empty()) {
        error = "personEmail is required";
        return false;
    }

    out.setPersonName(nameIt->second.get<std::string>());
    out.setPersonEmail(emailIt->second.get<std::string>());

    picojson::object::iterator addrIt = obj.find("personAddress");
    if (addrIt == obj.end() || addrIt->second.is<picojson::null>()) {
        out.setPersonAddressNull();
    } else if (addrIt->second.is<std::string>()) {
        out.setPersonAddress(addrIt->second.get<std::string>());
    } else {
        error = "personAddress must be a string or null";
        return false;
    }

    return true;
}

static picojson::value personToJson(const models::Person &person)
{
    picojson::object obj;
    obj["personId"] = picojson::value(static_cast<double>(person.personId()));
    obj["personName"] = picojson::value(person.personName());
    obj["personEmail"] = picojson::value(person.personEmail());
    if (!person.personAddressIsNull())
        obj["personAddress"] = picojson::value(person.personAddress());
    return picojson::value(obj);
}

void Person::writePerson(int status, const models::Person &person)
{
    response().status(status);
    response().content_type("application/json");
    response().out() << personToJson(person).serialize();
}

void Person::writePersonList(const std::vector<models::Person> &people)
{
    picojson::array arr;
    for (std::vector<models::Person>::const_iterator it = people.begin(); it != people.end(); ++it)
        arr.push_back(personToJson(*it));
    response().status(cppcms::http::response::ok);
    response().content_type("application/json");
    response().out() << picojson::value(arr).serialize();
}

void Person::writeError(int status, const std::string &message)
{
    picojson::object obj;
    obj["error"] = picojson::value(message);
    response().status(status);
    response().content_type("application/json");
    response().out() << picojson::value(obj).serialize();
}

void Person::collection()
{
    if (!requireAuth())
        return;

    std::string method = request().request_method();
    try {
        if (method == "GET") {
            writePersonList(personService_.getAll());
            return;
        }
        if (method == "POST") {
            models::Person person;
            std::string error;
            if (!parsePersonBody(person, error)) {
                writeError(cppcms::http::response::bad_request, error);
                return;
            }
            models::Person created = personService_.create(person);
            writePerson(cppcms::http::response::created, created);
            return;
        }
        writeError(cppcms::http::response::method_not_allowed, "Method not allowed");
    } catch (const DuplicateEmailError &e) {
        writeError(cppcms::http::response::conflict, e.what());
    } catch (const std::exception &e) {
        writeError(cppcms::http::response::internal_server_error, e.what());
    }
}

void Person::item(std::string id)
{
    if (!requireAuth())
        return;

    int personId = std::atoi(id.c_str());
    std::string method = request().request_method();
    try {
        if (method == "GET") {
            models::Person person;
            if (!personService_.getById(personId, person)) {
                writeError(cppcms::http::response::not_found, "Person not found");
                return;
            }
            writePerson(cppcms::http::response::ok, person);
            return;
        }
        if (method == "PUT") {
            models::Person person;
            std::string error;
            if (!parsePersonBody(person, error)) {
                writeError(cppcms::http::response::bad_request, error);
                return;
            }
            person.setPersonId(personId);
            if (!personService_.update(person)) {
                writeError(cppcms::http::response::not_found, "Person not found");
                return;
            }
            writePerson(cppcms::http::response::ok, person);
            return;
        }
        if (method == "DELETE") {
            if (!personService_.remove(personId)) {
                writeError(cppcms::http::response::not_found, "Person not found");
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
    } catch (const DuplicateEmailError &e) {
        writeError(cppcms::http::response::conflict, e.what());
    } catch (const std::exception &e) {
        writeError(cppcms::http::response::internal_server_error, e.what());
    }
}
