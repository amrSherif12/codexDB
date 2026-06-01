#include "test.h"

#include <iostream>
#include <string>

void test() {
    std::string red = "\033[1;31m";
    std::string green = "\033[1;32m";
    std::string reset = "\033[0m";

    int passed_tests = 0;
    int test_count = 0;

    std::cout << reset << "Basic CRUD queries test  " << red;
    if (query_test()) {
        passed_tests++;
        std::cout << green << "[TEST PASSED]\n";
    }
    test_count++;

    std::cout << reset << passed_tests << "/" << test_count << " tests passed";
}
