#include "person_controller_test.h"
#include "check.h"
#include "http_client.h"
#include <picojson.h>
#include <sstream>
#include <string>

static picojson::value parse_json(const std::string &body)
{
    picojson::value root;
    std::string err;
    picojson::parse(root, body.c_str(), body.c_str() + body.size(), &err);
    CHECK(err.empty());
    return root;
}

static int json_int(const picojson::value &v, const char *key)
{
    CHECK(v.contains(key));
    return static_cast<int>(v.get(key).get<double>());
}

static std::string json_str(const picojson::value &v, const char *key)
{
    CHECK(v.contains(key));
    CHECK(v.get(key).is<std::string>());
    return v.get(key).get<std::string>();
}

void test_person_happy_path()
{
    HttpResult listed = http_request("GET", "/person", TEST_TOKEN, 0);
    CHECK_EQ(listed.status, 200);
    picojson::value emptyList = parse_json(listed.body);
    CHECK(emptyList.is<picojson::array>());
    CHECK_EQ(emptyList.get<picojson::array>().size(), 0u);

    const char *createBody =
        "{\"personName\":\"Ada Lovelace\",\"personEmail\":\"ada@example.com\",\"personAddress\":\"London\"}";
    HttpResult created = http_request("POST", "/person", TEST_TOKEN, createBody);
    CHECK_EQ(created.status, 201);
    picojson::value createdJson = parse_json(created.body);
    int id = json_int(createdJson, "personId");
    CHECK(id > 0);
    CHECK_EQ(json_str(createdJson, "personName"), std::string("Ada Lovelace"));
    CHECK_EQ(json_str(createdJson, "personEmail"), std::string("ada@example.com"));
    CHECK_EQ(json_str(createdJson, "personAddress"), std::string("London"));

    std::ostringstream itemPath;
    itemPath << "/person/" << id;
    std::string path = itemPath.str();

    HttpResult got = http_request("GET", path.c_str(), TEST_TOKEN, 0);
    CHECK_EQ(got.status, 200);
    picojson::value gotJson = parse_json(got.body);
    CHECK_EQ(json_int(gotJson, "personId"), id);
    CHECK_EQ(json_str(gotJson, "personName"), std::string("Ada Lovelace"));

    HttpResult listAfter = http_request("GET", "/person", TEST_TOKEN, 0);
    CHECK_EQ(listAfter.status, 200);
    picojson::value arr = parse_json(listAfter.body);
    CHECK(arr.is<picojson::array>());
    CHECK_EQ(arr.get<picojson::array>().size(), 1u);
    CHECK_EQ(json_int(arr.get<picojson::array>()[0], "personId"), id);

    const char *updateBody =
        "{\"personName\":\"Ada Byron\",\"personEmail\":\"ada.byron@example.com\",\"personAddress\":null}";
    HttpResult updated = http_request("PUT", path.c_str(), TEST_TOKEN, updateBody);
    CHECK_EQ(updated.status, 200);
    picojson::value updatedJson = parse_json(updated.body);
    CHECK_EQ(json_int(updatedJson, "personId"), id);
    CHECK_EQ(json_str(updatedJson, "personName"), std::string("Ada Byron"));
    CHECK_EQ(json_str(updatedJson, "personEmail"), std::string("ada.byron@example.com"));
    CHECK(!updatedJson.contains("personAddress"));

    HttpResult gotAfterUpdate = http_request("GET", path.c_str(), TEST_TOKEN, 0);
    CHECK_EQ(gotAfterUpdate.status, 200);
    picojson::value gotAfterJson = parse_json(gotAfterUpdate.body);
    CHECK_EQ(json_int(gotAfterJson, "personId"), id);
    CHECK(!gotAfterJson.contains("personAddress"));

    HttpResult deleted = http_request("DELETE", path.c_str(), TEST_TOKEN, 0);
    CHECK_EQ(deleted.status, 200);
    picojson::value delJson = parse_json(deleted.body);
    CHECK(delJson.contains("ok"));
    CHECK(delJson.get("ok").is<bool>());
    CHECK(delJson.get("ok").get<bool>());
}

void test_person_unauthorized()
{
    HttpResult missing = http_request("GET", "/person", 0, 0);
    CHECK_EQ(missing.status, 401);
    picojson::value missingJson = parse_json(missing.body);
    CHECK_EQ(json_str(missingJson, "error"), std::string("Unauthorized"));

    HttpResult invalid = http_request("GET", "/person", "not-a-valid-token", 0);
    CHECK_EQ(invalid.status, 401);
    picojson::value invalidJson = parse_json(invalid.body);
    CHECK_EQ(json_str(invalidJson, "error"), std::string("Unauthorized"));

    HttpResult postNoAuth = http_request("POST", "/person", 0,
        "{\"personName\":\"X\",\"personEmail\":\"x@example.com\"}");
    CHECK_EQ(postNoAuth.status, 401);
}

void test_person_malformed()
{
    HttpResult empty = http_request("POST", "/person", TEST_TOKEN, "");
    CHECK_EQ(empty.status, 400);
    CHECK_EQ(json_str(parse_json(empty.body), "error"), std::string("JSON body is required"));

    HttpResult invalid = http_request("POST", "/person", TEST_TOKEN, "{");
    CHECK_EQ(invalid.status, 400);
    CHECK_EQ(json_str(parse_json(invalid.body), "error"), std::string("JSON Invalid"));

    HttpResult missingName = http_request("POST", "/person", TEST_TOKEN,
        "{\"personEmail\":\"n@example.com\"}");
    CHECK_EQ(missingName.status, 400);
    CHECK_EQ(json_str(parse_json(missingName.body), "error"), std::string("personName is required"));

    HttpResult emptyName = http_request("POST", "/person", TEST_TOKEN,
        "{\"personName\":\"\",\"personEmail\":\"n@example.com\"}");
    CHECK_EQ(emptyName.status, 400);
    CHECK_EQ(json_str(parse_json(emptyName.body), "error"), std::string("personName is required"));

    HttpResult missingEmail = http_request("POST", "/person", TEST_TOKEN,
        "{\"personName\":\"No Email\"}");
    CHECK_EQ(missingEmail.status, 400);
    CHECK_EQ(json_str(parse_json(missingEmail.body), "error"), std::string("personEmail is required"));

    HttpResult emptyEmail = http_request("POST", "/person", TEST_TOKEN,
        "{\"personName\":\"Name\",\"personEmail\":\"\"}");
    CHECK_EQ(emptyEmail.status, 400);
    CHECK_EQ(json_str(parse_json(emptyEmail.body), "error"), std::string("personEmail is required"));

    HttpResult badAddr = http_request("POST", "/person", TEST_TOKEN,
        "{\"personName\":\"Name\",\"personEmail\":\"n2@example.com\",\"personAddress\":1}");
    CHECK_EQ(badAddr.status, 400);
    CHECK_EQ(json_str(parse_json(badAddr.body), "error"),
             std::string("personAddress must be a string or null"));
}

void test_person_duplicate_email()
{
    const char *first =
        "{\"personName\":\"First\",\"personEmail\":\"dup@example.com\",\"personAddress\":\"A\"}";
    HttpResult created = http_request("POST", "/person", TEST_TOKEN, first);
    CHECK_EQ(created.status, 201);
    int firstId = json_int(parse_json(created.body), "personId");

    HttpResult dupPost = http_request("POST", "/person", TEST_TOKEN, first);
    CHECK_EQ(dupPost.status, 409);
    CHECK_EQ(json_str(parse_json(dupPost.body), "error"),
             std::string("personEmail already exists"));

    const char *second =
        "{\"personName\":\"Second\",\"personEmail\":\"other@example.com\"}";
    HttpResult created2 = http_request("POST", "/person", TEST_TOKEN, second);
    CHECK_EQ(created2.status, 201);
    int secondId = json_int(parse_json(created2.body), "personId");

    std::ostringstream path;
    path << "/person/" << secondId;
    const char *stealEmail =
        "{\"personName\":\"Second\",\"personEmail\":\"dup@example.com\"}";
    HttpResult dupPut = http_request("PUT", path.str().c_str(), TEST_TOKEN, stealEmail);
    CHECK_EQ(dupPut.status, 409);
    CHECK_EQ(json_str(parse_json(dupPut.body), "error"),
             std::string("personEmail already exists"));

    (void)firstId;
}

void test_person_accessing_deleted()
{
    const char *body =
        "{\"personName\":\"Gone\",\"personEmail\":\"gone@example.com\"}";
    HttpResult created = http_request("POST", "/person", TEST_TOKEN, body);
    CHECK_EQ(created.status, 201);
    int id = json_int(parse_json(created.body), "personId");

    std::ostringstream itemPath;
    itemPath << "/person/" << id;
    std::string path = itemPath.str();

    HttpResult deleted = http_request("DELETE", path.c_str(), TEST_TOKEN, 0);
    CHECK_EQ(deleted.status, 200);

    HttpResult got = http_request("GET", path.c_str(), TEST_TOKEN, 0);
    CHECK_EQ(got.status, 404);
    CHECK_EQ(json_str(parse_json(got.body), "error"), std::string("Person not found"));

    HttpResult put = http_request("PUT", path.c_str(), TEST_TOKEN, body);
    CHECK_EQ(put.status, 404);
    CHECK_EQ(json_str(parse_json(put.body), "error"), std::string("Person not found"));

    HttpResult delAgain = http_request("DELETE", path.c_str(), TEST_TOKEN, 0);
    CHECK_EQ(delAgain.status, 404);
    CHECK_EQ(json_str(parse_json(delAgain.body), "error"), std::string("Person not found"));
}
