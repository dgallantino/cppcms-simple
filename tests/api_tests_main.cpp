#include "check.h"
#include "http_client.h"
#include "person_controller_test.h"
#include "auth_controller_test.h"
#include "user_controller_test.h"
#include <controllers/Auth.h>
#include <controllers/Person.h>
#include <controllers/User.h>
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/json.h>
#include <cppcms/service.h>
#include <cppcms/url_mapper.h>
#include <cppdb/frontend.h>
#include <helpers/TokenManager.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

static cppdb::session *g_test_sql = 0;

class ApiTestApp : public cppcms::application
{
public:
    ApiTestApp(cppcms::service &srv) : cppcms::application(srv)
    {
        attach(new User(srv, *g_test_sql), "users", "/users/{1}",
               "/users(/(.*))?", 1);
        attach(new Auth(srv, *g_test_sql), "auth", "/auth/{1}",
               "/auth(/(.*))?", 1);
        attach(new Person(srv, *g_test_sql), "person", "/person/{1}",
               "/person(/(.*))?", 1);
        mapper().root("/");
    }
};

static void create_schema(cppdb::session &sql)
{
    sql << "CREATE TABLE IF NOT EXISTS \"person\" ("
           "\"personId\" INTEGER NOT NULL,"
           "\"personName\" TEXT NOT NULL,"
           "\"personEmail\" TEXT NOT NULL UNIQUE,"
           "\"personAddress\" TEXT,"
           "PRIMARY KEY(\"personId\" AUTOINCREMENT)"
           ")"
        << cppdb::exec;

    sql << "CREATE TABLE IF NOT EXISTS users ("
           "usrsId INTEGER PRIMARY KEY AUTOINCREMENT,"
           "usrsFirstName TEXT,"
           "usrsLastName TEXT,"
           "usrsLoginId TEXT,"
           "usrsLoginPass TEXT,"
           "usrsCreatedTime TIMESTAMP,"
           "usrsUpdatedTime TEXT,"
           "usrsIsBlocked INTEGER,"
           "usrsDeletedTime TIMESTAMP"
           ")"
        << cppdb::exec;

    sql << "INSERT INTO users (usrsFirstName, usrsLastName, usrsLoginId, usrsLoginPass, "
           "usrsCreatedTime, usrsUpdatedTime, usrsIsBlocked) "
           "VALUES ('Admin', 'Test', 'admin', 'admin1234', '2024-07-01 00:00:00', '', 0)"
        << cppdb::exec;
}

static cppcms::json::value test_settings()
{
    std::ostringstream ss;
    ss << "{"
       << "\"service\":{"
       << "\"api\":\"http\","
       << "\"ip\":\"127.0.0.1\","
       << "\"port\":" << TEST_HTTP_PORT << ","
       << "\"worker_threads\":1,"
       << "\"disable_global_exit_handling\":true"
       << "},"
       << "\"http\":{\"script_names\":[\"/\"]}"
       << "}";
    std::istringstream in(ss.str());
    cppcms::json::value conf;
    int line = 0;
    if (!conf.load(in, true, &line)) {
        std::cerr << "failed to parse test cppcms settings at line " << line << std::endl;
        std::exit(1);
    }
    return conf;
}

int main()
{
    cppdb::session sql("sqlite3:db=:memory:");
    try {
        create_schema(sql);
    } catch (const std::exception &e) {
        std::cerr << "failed to create :memory: schema: " << e.what() << std::endl;
        return 1;
    }
    g_test_sql = &sql;

    TokenManager::getInstance().clearAllTokens();
    TokenManager::getInstance().addToken(TEST_TOKEN, "{\"role\":\"test\"}");

    cppcms::json::value conf = test_settings();
    cppcms::service srv(conf);
    srv.applications_pool().mount(cppcms::applications_factory<ApiTestApp>());

    std::thread worker([&srv]() {
        try {
            srv.run();
        } catch (const std::exception &e) {
            std::cerr << "cppcms service: " << e.what() << std::endl;
        }
    });

    if (!wait_for_http_server(50, 20)) {
        std::cerr << "cppcms test server did not start on 127.0.0.1:" << TEST_HTTP_PORT << std::endl;
        srv.shutdown();
        worker.join();
        TokenManager::getInstance().clearAllTokens();
        return 1;
    }

    test_person_happy_path();
    test_person_unauthorized();
    test_person_malformed();
    test_person_duplicate_email();
    test_person_accessing_deleted();

    test_auth_login_logout();
    test_auth_invalid_and_malformed();

    test_user_happy_path();
    test_user_unauthorized();
    test_user_malformed();
    test_user_duplicate_login_id();
    test_user_accessing_deleted();

    srv.shutdown();
    worker.join();
    TokenManager::getInstance().clearAllTokens();
    g_test_sql = 0;

    if (g_failures > 0) {
        std::cerr << g_failures << " check(s) failed" << std::endl;
        return 1;
    }
    std::cout << "api_tests: all checks passed" << std::endl;
    return 0;
}
