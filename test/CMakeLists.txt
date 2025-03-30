set(TRGT_TEST lea-tests)

enable_testing()

find_package(GTest REQUIRED)

set(TEST_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/keyhash.test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cipher.test.cpp
)

add_executable(${TRGT_TEST} ${TEST_SOURCES})
target_link_libraries(${TRGT_TEST} PRIVATE ${TRGT_CORE} GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${TRGT_TEST})

log_target_info(${TRGT_TEST})