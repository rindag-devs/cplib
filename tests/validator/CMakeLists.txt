function(add_validator_test test_name)
  set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${test_name}")
  set(TARGET_NAME "val_${test_name}")

  add_executable(${TARGET_NAME} "${TEST_DIR}/val.cpp")
  target_link_libraries(${TARGET_NAME} PRIVATE cplib)

  file(GLOB TEST_CASES "${TEST_DIR}/data/*.in")

  foreach(IN_FILE ${TEST_CASES})
    get_filename_component(CASE_NAME ${IN_FILE} NAME_WE)
    set(JSON_FILE "${TEST_DIR}/data/${CASE_NAME}.json")

    add_test(
      NAME "validator.${test_name}.${CASE_NAME}"
      COMMAND
        ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/run_test_case.py simple
        --expected-stderr ${JSON_FILE} --stdin ${IN_FILE} --
        $<TARGET_FILE:${TARGET_NAME}> --report-format=json
        --reader-trace-level=2)
  endforeach()
endfunction()

add_validator_test(0-i32-a-plus-b)
add_validator_test(1-undirected-graph)
