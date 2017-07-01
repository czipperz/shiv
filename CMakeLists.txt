cmake_minimum_required(VERSION 2.6)

project(SHIV)

add_subdirectory(cutil)

set(files
  ${SHIV_SOURCE_DIR}/src/arguments.c
  ${SHIV_SOURCE_DIR}/src/diagnostics.c
  ${SHIV_SOURCE_DIR}/src/fiter.c
  ${SHIV_SOURCE_DIR}/src/lex.c
  ${SHIV_SOURCE_DIR}/src/main.c
  ${SHIV_SOURCE_DIR}/src/parse.c
  )
add_executable(shiv ${files})
target_link_libraries(shiv cutil)

add_executable(test_shiv ${files})
target_link_libraries(test_shiv cutil)
target_compile_definitions(test_shiv PRIVATE "TEST_MODE")

target_compile_options(shiv PRIVATE "-Wall" "-Wextra")
target_compile_options(test_shiv PRIVATE "-Wall" "-Wextra")
