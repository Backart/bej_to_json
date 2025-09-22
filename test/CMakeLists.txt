cmake_minimum_required(VERSION 3.10)
project(bej_to_json_tests)

enable_language(CXX)

find_package(GTest REQUIRED)

set(TEST_SOURCES
    test_main.cpp
    test_bej_parser.cpp
    test_json_writer.cpp
    ../src/bej_parser.c
    ../src/json_writer.c
)

add_executable(bej_tests ${TEST_SOURCES})

target_include_directories(bej_tests PRIVATE
    ../src
    ../include
)

target_link_libraries(bej_tests GTest::gtest GTest::gtest_main)

gtest_discover_tests(bej_tests)