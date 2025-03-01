#
# smbase tests CMakeLists.txt
#

# tests
project(nonport)
project(bit2d)
project(str)
project(strutil)
project(trdelete)
project(bflatten)
project(tobjpool)
project(cycles)
project(crc)
project(srcloc)
project(hashline)
project(gprintf)
project(autofile)

# files for nonport
add_executable(nonport
    ../gprintf.c
    ../nonport.cpp
)

# files for bit2d
add_executable(bit2d
    ../bit2d.cc
)

# files for str
add_executable(str
    ../str.cpp
)

# files for strutil
add_executable(strutil
    ../strutil.cc
)

# files for trdelete
add_executable(trdelete
    ../trdelete.cc
)

# files for bflatten
add_executable(bflatten
    ../bflatten.cc
)

# files for tobjpool
add_executable(tobjpool
    ../tobjpool.cc
)

# files for cycles
add_executable(cycles
    ../cycles.c
)

# files for crc
add_executable(crc
    ../crc.cpp
)

# files for srcloc
add_executable(srcloc
    ../srcloc.cc
)

# files for hashline
add_executable(hashline
    ../hashline.cc
)

# files for gprintf
add_executable(gprintf
    ../gprintf.c
)

# files for autofile
add_executable(autofile
    ../autofile.cc
)

# extra compile options
target_compile_options(nonport PRIVATE -DTEST_NONPORT)
target_compile_options(bit2d PRIVATE -DTEST_BIT2D)
target_compile_options(str PRIVATE -DTEST_STR)
target_compile_options(strutil PRIVATE -DTEST_STRUTIL)
target_compile_options(trdelete PRIVATE -DTEST_TRDELETE)
target_compile_options(bflatten PRIVATE -DTEST_BFLATTEN)
target_compile_options(cycles PRIVATE -DTEST_CYCLES)
target_compile_options(crc PRIVATE -DTEST_CRC)
target_compile_options(srcloc PRIVATE -DTEST_SRCLOC)
target_compile_options(hashline PRIVATE -DTEST_HASHLINE)
target_compile_options(gprintf PRIVATE -DTEST_GPRINTF)
target_compile_options(autofile PRIVATE -DTEST_AUTOFILE)

# link options
target_link_libraries(bit2d smbase)
target_link_libraries(str smbase)
target_link_libraries(strutil smbase)
target_link_libraries(trdelete smbase)
target_link_libraries(bflatten smbase)
target_link_libraries(tobjpool smbase)
target_link_libraries(srcloc smbase)
target_link_libraries(hashline smbase)
target_link_libraries(autofile smbase)

# add the tests
add_test(NAME nonport COMMAND ./nonport)
add_test(NAME bit2d COMMAND ./bit2d)
add_test(NAME str COMMAND ./str)
add_test(NAME strutil COMMAND ./strutil)
add_test(NAME trdelete COMMAND ./trdelete)
add_test(NAME bflatten COMMAND ./bflatten)
add_test(NAME tobjpool COMMAND ./tobjpool)
add_test(NAME cycles COMMAND ./cycles)
add_test(NAME crc COMMAND ./crc)
add_test(NAME srcloc COMMAND ./srcloc)
add_test(NAME hashline COMMAND ./hashline)
add_test(NAME gprintf COMMAND ./gprintf)
add_test(NAME autofile COMMAND ./autofile)

# dependencies for srcloc test
add_custom_command(TARGET srcloc POST_BUILD
  COMMAND ${CMAKE_C_COMPILER} -E -DTEST_SRCLOC srcloc.test.cc >${CMAKE_CURRENT_BINARY_DIR}/srcloc.tmp
  COMMAND ${CMAKE_COMMAND} -E copy srcloc.cc ${CMAKE_CURRENT_BINARY_DIR}/srcloc.cc
  COMMAND ${CMAKE_COMMAND} -E copy srcloc.h ${CMAKE_CURRENT_BINARY_DIR}/srcloc.h
  COMMAND ${CMAKE_COMMAND} -E copy srcloc.test.cc ${CMAKE_CURRENT_BINARY_DIR}/srcloc.test.cc
  COMMAND ${CMAKE_COMMAND} -E copy srcloc.test2.cc ${CMAKE_CURRENT_BINARY_DIR}/srcloc.test2.cc
  DEPENDS srcloc.test.cc
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/srcloc.cc
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/srcloc.h
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/srcloc.test.cc
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/srcloc.test2.cc
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../)
