include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

file(GLOB UNIT_TEST_SOURCES "unit/*.cpp")

add_executable(cplib_unit_tests ${UNIT_TEST_SOURCES})
target_link_libraries(cplib_unit_tests PRIVATE cplib gtest_main)

include(GoogleTest)
gtest_discover_tests(cplib_unit_tests)

add_subdirectory(checker)
add_subdirectory(validator)
add_subdirectory(generator)
add_subdirectory(interactor)
