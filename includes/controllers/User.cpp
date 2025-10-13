#include "User.h"
#include <cppcms/service.h>  
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/http_response.h>
#include <cppcms/http_request.h>
#include "helpers/TokenManager.h"

// constructor
User::User(cppcms::service &srv) : Master(srv)
{
    dispatcher().assign("", &User::getUser, this);
    mapper().assign("");

    dispatcher().assign("/(\\d+)", &User::getUserById, this, 1);
    mapper().assign("/{1}");
}

void User::getUser()
{
    // just an example to set token
    std::string token = request().getenv("HTTP_TOKEN");

    TokenManager& tokenManager = TokenManager::getInstance();

    // Add token with JSON data (example user data)
    std::string userData = "{\"userId\":\"123\",\"username\":\"testuser\",\"role\":\"user\"}";
    tokenManager.addToken(token, userData, std::chrono::minutes(1));
    response().out() <<"get all users: hdr = " << token;
}

void User::getUserById(std::string id)
{
    TokenManager& tokenManager = TokenManager::getInstance();
    std::string token = request().getenv("HTTP_TOKEN");
    // Check if token is valid
    if (tokenManager.isValidToken(token)) {
        // Token is valid and not expired
        response().out() << "Valid";

        std::string dataFromToken = tokenManager.getTokenData(token);
        response().out() << ", Data : " << dataFromToken;
    }
    response().out() <<"get user by id "<< id;
}
