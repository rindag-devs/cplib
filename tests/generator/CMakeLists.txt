function(add_generator_test test_name)
  set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${test_name}")
  set(TARGET_NAME "gen_${test_name}")

  add_executable(${TARGET_NAME} "${TEST_DIR}/gen.cpp")
  target_link_libraries(${TARGET_NAME} PRIVATE cplib)

  file(GLOB TEST_CASES "${TEST_DIR}/data/*.args")

  foreach(ARGS_FILE ${TEST_CASES})
    get_filename_component(CASE_NAME ${ARGS_FILE} NAME_WE)
    set(OUT_FILE "${TEST_DIR}/data/${CASE_NAME}.in")

    file(READ ${ARGS_FILE} GEN_ARGS)
    string(REPLACE "\n" ";" GEN_ARGS_LIST ${GEN_ARGS})

    add_test(
      NAME "generator.${test_name}.${CASE_NAME}"
      COMMAND
        ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/run_test_case.py simple
        --expected-stdout ${OUT_FILE} -- $<TARGET_FILE:${TARGET_NAME}>
        ${GEN_ARGS_LIST})
  endforeach()
endfunction()

add_generator_test(0-i32-a-plus-b)
