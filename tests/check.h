#ifndef TESTS_CHECK_H
#define TESTS_CHECK_H

#include <iostream>
#include <string>

static int g_failures = 0;

#define CHECK(cond) do { \
    if (!(cond)) { \
        std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ \
                  << ": " << #cond << std::endl; \
        ++g_failures; \
    } \
} while (0)

#define CHECK_EQ(a, b) do { \
    if (!((a) == (b))) { \
        std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ \
                  << ": " << #a << " == " << #b \
                  << " (" << (a) << " != " << (b) << ")" << std::endl; \
        ++g_failures; \
    } \
} while (0)

#endif
