#include "user_controller_test.h"
#include "check.h"
#include "http_client.h"
#include "json_util.h"
#include <sstream>
#include <string>

void test_user_happy_path()
{
    HttpResult listed = http_request("GET", "/users", TEST_TOKEN, 0);
    CHECK_EQ(listed.status, 200);
    picojson::value listJson = parse_json(listed.body);
    CHECK(listJson.is<picojson::array>());
    size_t before = listJson.get<picojson::array>().size();
    CHECK(before >= 1u);

    const char *createBody =
        "{\"usrsFirstName\":\"Ada\",\"usrsLastName\":\"Lovelace\",\"usrsLoginId\":\"ada\",\"usrsLoginPass\":\"ada1234\"}";
    HttpResult created = http_request("POST", "/users", TEST_TOKEN, createBody);
    CHECK_EQ(created.status, 201);
    picojson::value createdJson = parse_json(created.body);
    int id = json_int(createdJson, "usrsId");
    CHECK(id > 0);
    CHECK_EQ(json_str(createdJson, "usrsFirstName"), std::string("Ada"));
    CHECK_EQ(json_str(createdJson, "usrsLastName"), std::string("Lovelace"));
    CHECK_EQ(json_str(createdJson, "usrsLoginId"), std::string("ada"));
    CHECK_EQ(json_str(createdJson, "usrsLoginPass"), std::string("ada1234"));
    CHECK(!json_str(createdJson, "usrsCreatedTime").empty());

    std::ostringstream itemPath;
    itemPath << "/users/" << id;
    std::string path = itemPath.str();

    HttpResult got = http_request("GET", path.c_str(), TEST_TOKEN, 0);
    CHECK_EQ(got.status, 200);
    CHECK_EQ(json_int(parse_json(got.body), "usrsId"), id);

    HttpResult listAfter = http_request("GET", "/users", TEST_TOKEN, 0);
    CHECK_EQ(listAfter.status, 200);
    CHECK_EQ(parse_json(listAfter.body).get<picojson::array>().size(), before + 1);

    const char *updateBody =
        "{\"usrsFirstName\":\"Ada\",\"usrsLastName\":\"Byron\",\"usrsLoginId\":\"ada\",\"usrsLoginPass\":\"newpass\"}";
    HttpResult updated = http_request("PUT", path.c_str(), TEST_TOKEN, updateBody);
    CHECK_EQ(updated.status, 200);
    picojson::value updatedJson = parse_json(updated.body);
    CHECK_EQ(json_str(updatedJson, "usrsLastName"), std::string("Byron"));
    CHECK_EQ(json_str(updatedJson, "usrsLoginPass"), std::string("newpass"));

    HttpResult deleted = http_request("DELETE", path.c_str(), TEST_TOKEN, 0);
    CHECK_EQ(deleted.status, 200);
    CHECK(parse_json(deleted.body).get("ok").get<bool>());
}

void test_user_unauthorized()
{
    HttpResult missing = http_request("GET", "/users", 0, 0);
    CHECK_EQ(missing.status, 401);
    CHECK_EQ(json_str(parse_json(missing.body), "error"), std::string("Unauthorized"));

    HttpResult invalid = http_request("GET", "/users", "not-a-valid-token", 0);
    CHECK_EQ(invalid.status, 401);

    HttpResult postNoAuth = http_request("POST", "/users", 0,
        "{\"usrsLoginId\":\"x\",\"usrsLoginPass\":\"y\"}");
    CHECK_EQ(postNoAuth.status, 401);
}

void test_user_malformed()
{
    HttpResult empty = http_request("POST", "/users", TEST_TOKEN, "");
    CHECK_EQ(empty.status, 400);
    CHECK_EQ(json_str(parse_json(empty.body), "error"), std::string("JSON body is required"));

    HttpResult invalid = http_request("POST", "/users", TEST_TOKEN, "{");
    CHECK_EQ(invalid.status, 400);
    CHECK_EQ(json_str(parse_json(invalid.body), "error"), std::string("JSON Invalid"));

    HttpResult missingLogin = http_request("POST", "/users", TEST_TOKEN,
        "{\"usrsLoginPass\":\"x\"}");
    CHECK_EQ(missingLogin.status, 400);
    CHECK_EQ(json_str(parse_json(missingLogin.body), "error"), std::string("usrsLoginId is required"));

    HttpResult missingPass = http_request("POST", "/users", TEST_TOKEN,
        "{\"usrsLoginId\":\"x\"}");
    CHECK_EQ(missingPass.status, 400);
    CHECK_EQ(json_str(parse_json(missingPass.body), "error"), std::string("usrsLoginPass is required"));
}

void test_user_duplicate_login_id()
{
    const char *first =
        "{\"usrsFirstName\":\"One\",\"usrsLastName\":\"A\",\"usrsLoginId\":\"dupuser\",\"usrsLoginPass\":\"p1\"}";
    HttpResult created = http_request("POST", "/users", TEST_TOKEN, first);
    CHECK_EQ(created.status, 201);

    HttpResult dupPost = http_request("POST", "/users", TEST_TOKEN, first);
    CHECK_EQ(dupPost.status, 409);
    CHECK_EQ(json_str(parse_json(dupPost.body), "error"),
             std::string("usrsLoginId already exists"));

    const char *second =
        "{\"usrsFirstName\":\"Two\",\"usrsLastName\":\"B\",\"usrsLoginId\":\"otheruser\",\"usrsLoginPass\":\"p2\"}";
    HttpResult created2 = http_request("POST", "/users", TEST_TOKEN, second);
    CHECK_EQ(created2.status, 201);
    int secondId = json_int(parse_json(created2.body), "usrsId");

    std::ostringstream path;
    path << "/users/" << secondId;
    const char *steal =
        "{\"usrsFirstName\":\"Two\",\"usrsLastName\":\"B\",\"usrsLoginId\":\"dupuser\",\"usrsLoginPass\":\"p2\"}";
    HttpResult dupPut = http_request("PUT", path.str().c_str(), TEST_TOKEN, steal);
    CHECK_EQ(dupPut.status, 409);
    CHECK_EQ(json_str(parse_json(dupPut.body), "error"),
             std::string("usrsLoginId already exists"));
}

void test_user_accessing_deleted()
{
    const char *body =
        "{\"usrsFirstName\":\"Gone\",\"usrsLastName\":\"User\",\"usrsLoginId\":\"goneuser\",\"usrsLoginPass\":\"x\"}";
    HttpResult created = http_request("POST", "/users", TEST_TOKEN, body);
    CHECK_EQ(created.status, 201);
    int id = json_int(parse_json(created.body), "usrsId");

    std::ostringstream itemPath;
    itemPath << "/users/" << id;
    std::string path = itemPath.str();

    HttpResult deleted = http_request("DELETE", path.c_str(), TEST_TOKEN, 0);
    CHECK_EQ(deleted.status, 200);

    HttpResult got = http_request("GET", path.c_str(), TEST_TOKEN, 0);
    CHECK_EQ(got.status, 404);
    CHECK_EQ(json_str(parse_json(got.body), "error"), std::string("User not found"));

    HttpResult put = http_request("PUT", path.c_str(), TEST_TOKEN, body);
    CHECK_EQ(put.status, 404);

    HttpResult delAgain = http_request("DELETE", path.c_str(), TEST_TOKEN, 0);
    CHECK_EQ(delAgain.status, 404);
}
