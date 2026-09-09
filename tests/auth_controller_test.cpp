#include "auth_controller_test.h"
#include "check.h"
#include "http_client.h"
#include "json_util.h"
#include <string>

void test_auth_login_logout()
{
    const char *body = "{\"User\":{\"LoginId\":\"admin\",\"Password\":\"admin1234\"}}";
    HttpResult login = http_request("POST", "/auth/login", 0, body);
    CHECK_EQ(login.status, 200);
    picojson::value loginJson = parse_json(login.body);
    std::string token = json_str(loginJson, "token");
    CHECK(!token.empty());

    HttpResult listed = http_request("GET", "/person", token.c_str(), 0);
    CHECK_EQ(listed.status, 200);

    HttpResult logout = http_request("POST", "/auth/logout", token.c_str(), 0);
    CHECK_EQ(logout.status, 200);
    picojson::value logoutJson = parse_json(logout.body);
    CHECK(logoutJson.contains("ok"));
    CHECK(logoutJson.get("ok").get<bool>());

    HttpResult after = http_request("GET", "/person", token.c_str(), 0);
    CHECK_EQ(after.status, 401);
    CHECK_EQ(json_str(parse_json(after.body), "error"), std::string("Unauthorized"));

    HttpResult logoutAgain = http_request("POST", "/auth/logout", token.c_str(), 0);
    CHECK_EQ(logoutAgain.status, 401);
}

void test_auth_invalid_and_malformed()
{
    HttpResult badPass = http_request("POST", "/auth/login", 0,
        "{\"User\":{\"LoginId\":\"admin\",\"Password\":\"wrong\"}}");
    CHECK_EQ(badPass.status, 401);
    CHECK_EQ(json_str(parse_json(badPass.body), "error"), std::string("Login invalid"));

    HttpResult unknown = http_request("POST", "/auth/login", 0,
        "{\"User\":{\"LoginId\":\"nobody\",\"Password\":\"admin1234\"}}");
    CHECK_EQ(unknown.status, 401);

    HttpResult empty = http_request("POST", "/auth/login", 0, "");
    CHECK_EQ(empty.status, 400);
    CHECK_EQ(json_str(parse_json(empty.body), "error"), std::string("JSON body is required"));

    HttpResult invalid = http_request("POST", "/auth/login", 0, "{");
    CHECK_EQ(invalid.status, 400);
    CHECK_EQ(json_str(parse_json(invalid.body), "error"), std::string("JSON Invalid"));

    HttpResult getLogin = http_request("GET", "/auth/login", 0, 0);
    CHECK_EQ(getLogin.status, 405);

    HttpResult logoutNone = http_request("POST", "/auth/logout", 0, 0);
    CHECK_EQ(logoutNone.status, 401);
}
