function(add_checker_test test_name)
  set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${test_name}")
  set(TARGET_NAME "chk_${test_name}")

  add_executable(${TARGET_NAME} "${TEST_DIR}/chk.cpp")
  target_link_libraries(${TARGET_NAME} PRIVATE cplib)

  file(GLOB TEST_CASES "${TEST_DIR}/data/*.in")

  foreach(IN_FILE ${TEST_CASES})
    get_filename_component(CASE_NAME ${IN_FILE} NAME_WE)
    set(OUT_FILE "${TEST_DIR}/data/${CASE_NAME}.out")
    set(ANS_FILE "${TEST_DIR}/data/${CASE_NAME}.ans")
    set(JSON_FILE "${TEST_DIR}/data/${CASE_NAME}.json")

    add_test(
      NAME "checker.${test_name}.${CASE_NAME}"
      COMMAND
        ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/run_test_case.py simple
        --expected-stderr ${JSON_FILE} -- $<TARGET_FILE:${TARGET_NAME}>
        ${IN_FILE} ${OUT_FILE} ${ANS_FILE} --report-format=json)
  endforeach()
endfunction()

add_checker_test(0-i32-a-plus-b)
add_checker_test(1-f64-a-plus-b-multiple)
add_checker_test(2-shortest-path)
add_checker_test(3-set-of-sets)
