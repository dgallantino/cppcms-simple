#ifndef TESTS_USER_CONTROLLER_TEST_H
#define TESTS_USER_CONTROLLER_TEST_H

void test_user_happy_path();
void test_user_unauthorized();
void test_user_malformed();
void test_user_duplicate_login_id();
void test_user_accessing_deleted();

#endif
