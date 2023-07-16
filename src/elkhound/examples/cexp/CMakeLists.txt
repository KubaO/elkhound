#
# cexp CMakeLists.txt
#
project(cexp)

# generate cexp3ast.ast.gen.{cc,h}
add_custom_command(
    OUTPUT cexp3ast.ast.gen.cc cexp3ast.ast.gen.h
    COMMAND astgen -bcexp3ast.ast.gen ${CMAKE_CURRENT_SOURCE_DIR}/cexp3ast.ast
    DEPENDS astgen cexp3ast.ast
)

# gemerate cexp3.gr.gen.{cc.h}
add_custom_command(
    OUTPUT cexp3.gr.gen.cc cexp3.gr.gen.h
    COMMAND elkhound -o cexp3.gr.gen ${CMAKE_CURRENT_SOURCE_DIR}/cexp3.gr
    DEPENDS elkhound cexp3.gr
)

# all the files for cexp
add_executable(cexp
    cexp3.gr.gen
    cexp3ast.ast.gen
    ../../glrmain.cc
    cexp3mrg.cc
)

# link against elkhound, smbase, libcparse and ast
target_link_libraries(cexp libelkhound smbase libcparse ast)

