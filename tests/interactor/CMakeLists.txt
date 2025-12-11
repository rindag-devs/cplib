function(add_interactor_test test_name)
  set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${test_name}")
  set(INTERACTOR_TARGET "intr_${test_name}")
  set(SOLUTION_TARGET "std_${test_name}")

  add_executable(${INTERACTOR_TARGET} "${TEST_DIR}/intr.cpp")
  target_link_libraries(${INTERACTOR_TARGET} PRIVATE cplib)

  add_executable(${SOLUTION_TARGET} "${TEST_DIR}/std.cpp")
  target_link_libraries(${SOLUTION_TARGET} PRIVATE cplib)

  file(GLOB TEST_CASES "${TEST_DIR}/data/*.in")

  foreach(IN_FILE ${TEST_CASES})
    get_filename_component(CASE_NAME ${IN_FILE} NAME_WE)
    set(JSON_FILE "${TEST_DIR}/data/${CASE_NAME}.json")

    add_test(
      NAME "interactor.${test_name}.${CASE_NAME}"
      COMMAND
        ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/run_test_case.py
        interactive --solution $<TARGET_FILE:${SOLUTION_TARGET}>
        --expected-stderr ${JSON_FILE} -- $<TARGET_FILE:${INTERACTOR_TARGET}>
        ${IN_FILE} --report-format=json)
  endforeach()
endfunction()

add_interactor_test(0-guessing-number)
