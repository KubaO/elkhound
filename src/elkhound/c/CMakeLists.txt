#
# cparse CMakeLists.txt
#
project(libcparse)
project(cparse)

# tests
project(clexer1)
project(clexer2)

# generate lexer1yy.cc
add_custom_command(
    OUTPUT lexer1yy.cc
    COMMAND ${FLEX_EXECUTABLE} -olexer1yy.cc ${CMAKE_CURRENT_SOURCE_DIR}/lexer1.lex
    MAIN_DEPENDENCY lexer1.lex
)

# generate c.ast.gen.{cc,h}
add_custom_command(
    OUTPUT c.ast.gen.cc c.ast.gen.h
    COMMAND astgen -oc.ast.gen ${CMAKE_CURRENT_SOURCE_DIR}/c.ast
    DEPENDS astgen c.ast
)

# generate c.gr.{cc,h}
add_custom_command(
    OUTPUT c.gr.gen.cc c.gr.gen.h
    COMMAND elkhound -tr bison -o c.gr.gen ${CMAKE_CURRENT_SOURCE_DIR}/c.gr
    DEPENDS elkhound c.tok c.gr
)

# generate c.tok
add_custom_command(
    OUTPUT c.tok
    COMMAND lexer2 -myparser > c.tok
    DEPENDS lexer2
)

# all the files for libcparse
add_library(libcparse STATIC
    c.ast.gen.cc
    c.gr.gen.cc
    c_env.cc
    c_type.cc
    c_variable.cc
    cc_flags.cc
    cc_lang.cc
    cparse.cc
    exprequal.cc
    exprvisit.cc
    lexer1.cc
    lexer1yy.cc
    lexer2.cc
    parssppt.cc
    paths.cc
    postorder.cc
    stubs.cc
    tcheck.cc
    treeout.cc
)
# all the files for cparse
add_executable(cparse
    main.cc
)

# tests

# all the files for clexer1
add_executable(clexer1
    lexer1.cc
    lexer1yy.cc
)

# all the files for clexer2
add_executable(clexer2
    lexer2.cc
    lexer1.cc
    lexer1yy.cc
    cc_lang.cc
)

# extra compile options
target_compile_options(clexer1 PRIVATE -DTEST_LEXER1)
target_compile_options(clexer2 PRIVATE -DTEST_LEXER2)

# suppress -Wsign-compare for Flex-generated code
if (NOT MSVC)
    SET_SOURCE_FILES_PROPERTIES( lexer1yy.cc PROPERTIES COMPILE_FLAGS -Wno-sign-compare )
endif()

# link options
target_link_libraries(libcparse smbase ast libelkhound)
target_link_libraries(cparse libcparse)

target_link_libraries(clexer1 smbase ast libelkhound)
target_link_libraries(clexer2 smbase ast libelkhound)

# disable lib prefix for libelkhound
SET_TARGET_PROPERTIES(libcparse PROPERTIES PREFIX "")

# add the tests
if(BUILD_TESTING)
    add_test (NAME clexer1 COMMAND ./clexer1 ${CMAKE_CURRENT_SOURCE_DIR}/../c.in/c.in4)
    add_test (NAME clexer2 COMMAND ./clexer2 ${CMAKE_CURRENT_SOURCE_DIR}/../c.in/c.in4)
endif()
