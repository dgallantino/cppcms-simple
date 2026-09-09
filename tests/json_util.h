#ifndef TESTS_JSON_UTIL_H
#define TESTS_JSON_UTIL_H

#include "check.h"
#include <picojson.h>
#include <string>

inline picojson::value parse_json(const std::string &body)
{
    picojson::value root;
    std::string err;
    picojson::parse(root, body.c_str(), body.c_str() + body.size(), &err);
    CHECK(err.empty());
    return root;
}

inline int json_int(const picojson::value &v, const char *key)
{
    CHECK(v.contains(key));
    return static_cast<int>(v.get(key).get<double>());
}

inline std::string json_str(const picojson::value &v, const char *key)
{
    CHECK(v.contains(key));
    CHECK(v.get(key).is<std::string>());
    return v.get(key).get<std::string>();
}

#endif
