#include "check.h"
#include "http_client.h"
#include "person_controller_test.h"
#include <controllers/Person.h>
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/json.h>
#include <cppcms/service.h>
#include <cppcms/url_mapper.h>
#include <cppdb/frontend.h>
#include <helpers/TokenManager.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#ifndef TEST_DB_PATH
#define TEST_DB_PATH "test.db"
#endif

class PersonTestApp : public cppcms::application
{
public:
    PersonTestApp(cppcms::service &srv) : cppcms::application(srv)
    {
        attach(new Person(srv), "person", "/person/{1}",
               "/person(/(.*))?", 1);
        mapper().root("/");
    }
};

static void remove_if_exists(const std::string &path)
{
    std::remove(path.c_str());
}

static void recreate_test_db(const std::string &path)
{
    remove_if_exists(path);
    remove_if_exists(path + "-wal");
    remove_if_exists(path + "-shm");

    cppdb::session sql(std::string("sqlite3:db=") + path);
    sql << "CREATE TABLE IF NOT EXISTS \"person\" ("
           "\"personId\" INTEGER NOT NULL,"
           "\"personName\" TEXT NOT NULL,"
           "\"personEmail\" TEXT NOT NULL UNIQUE,"
           "\"personAddress\" TEXT,"
           "PRIMARY KEY(\"personId\" AUTOINCREMENT)"
           ")"
        << cppdb::exec;
}

static cppcms::json::value test_settings(const std::string &dbPath)
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
       << "\"http\":{\"script_names\":[\"/\"]},"
       << "\"cppcms_simple\":{\"connection_string\":\"sqlite3:db=" << dbPath << "\"}"
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
    const std::string dbPath = TEST_DB_PATH;
    try {
        recreate_test_db(dbPath);
    } catch (const std::exception &e) {
        std::cerr << "failed to create " << dbPath << ": " << e.what() << std::endl;
        return 1;
    }

    TokenManager::getInstance().clearAllTokens();
    TokenManager::getInstance().addToken(TEST_TOKEN, "{\"role\":\"test\"}");

    cppcms::json::value conf = test_settings(dbPath);
    cppcms::service srv(conf);
    srv.applications_pool().mount(cppcms::applications_factory<PersonTestApp>());

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

    srv.shutdown();
    worker.join();
    TokenManager::getInstance().clearAllTokens();

    if (g_failures > 0) {
        std::cerr << g_failures << " check(s) failed" << std::endl;
        return 1;
    }
    std::cout << "person_tests: all checks passed" << std::endl;
    return 0;
}
