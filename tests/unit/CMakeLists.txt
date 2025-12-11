file(GLOB_RECURSE UNIT_TEST_SOURCES
  "cmd/*.cpp"
  "evaluate/*.cpp"
  "io/*.cpp"
  "json/*.cpp"
  "random/*.cpp"
  "utils/*.cpp"
  "var/*.cpp"
)

add_executable(unit ${UNIT_TEST_SOURCES})

target_link_libraries(unit PRIVATE cplib gtest_main)
target_include_directories(unit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(unit TEST_PREFIX "unit.")
