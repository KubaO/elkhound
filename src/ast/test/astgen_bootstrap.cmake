# This is a script that tests astgen bootstrap.
# It compares the ast.ast[.cc,.h] files in the sources with those generated from ast.ast by astgen.
# Usage:
#   ${CMAKE_COMMAND} -P astgen_bootstrap.cmake -- ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
# CMAKE_ARGV  0       1           2             3              4                            5

set (BINARY_DIR "${CMAKE_ARGV4}")
set (SOURCE_DIR "${CMAKE_ARGV5}")

file (TO_NATIVE_PATH "${BINARY_DIR}" BINARY_NATIVE_DIR)
message ("Output in: ${BINARY_NATIVE_DIR}")

execute_process(
	COMMAND ${BINARY_DIR}/../astgen -oast.ast ${SOURCE_DIR}/../ast.ast
	WORKING_DIRECTORY ${BINARY_DIR}
	COMMAND_ERROR_IS_FATAL ANY
)

execute_process(
	COMMAND ${CMAKE_COMMAND} -E compare_files ${BINARY_DIR}/ast.ast.h  ${SOURCE_DIR}/../ast.ast.h
	WORKING_DIRECTORY ${BINARY_DIR}
	COMMAND_ERROR_IS_FATAL ANY
)

execute_process(
	COMMAND ${CMAKE_COMMAND} -E compare_files ${BINARY_DIR}/ast.ast.cc ${SOURCE_DIR}/../ast.ast.cc
	WORKING_DIRECTORY ${BINARY_DIR}
	COMMAND_ERROR_IS_FATAL ANY
)
