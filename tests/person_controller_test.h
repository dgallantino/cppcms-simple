#ifndef TESTS_PERSON_CONTROLLER_TEST_H
#define TESTS_PERSON_CONTROLLER_TEST_H

void test_person_happy_path();
void test_person_unauthorized();
void test_person_malformed();
void test_person_duplicate_email();
void test_person_accessing_deleted();

#endif
