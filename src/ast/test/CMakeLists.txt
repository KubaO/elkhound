#
# ast tests CMakeLists.txt
#

# tests
project(ccsstr)

# files for ccsstr
add_executable(ccsstr
    ../ccsstr.cc
    ../embedded.cc
    ../reporterr.cc
)

# extra compile options
target_compile_options(ccsstr PRIVATE -DTEST_CCSSTR)

# link options
target_link_libraries(ccsstr smbase)

# add tests
add_test(NAME ccsstr COMMAND ./ccsstr)

add_test(
    NAME astgen_bootstrap
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/astgen_bootstrap.cmake -- ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
)
