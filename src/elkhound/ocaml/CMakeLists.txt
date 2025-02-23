#
# ocaml CMakeLists.txt
#
project(ocaml_main)

find_package(OCaml)

if(NOT OCAMLFIND)
    message(WARNING " * Skipping the ocaml tests: ocaml not found")
    return()
endif()

find_program(OCAMLC_EXECUTABLE ocamlc HINTS "${OCAML_BINDIR}")
find_program(OCAMLRUN_EXECUTABLE ocamlrun HINTS "${OCAML_BINDIR}")
find_program(OCAMLLEX_EXECUTABLE ocamllex HINTS "${OCAML_BINDIR}")
find_program(OCAMLYACC_EXECUTABLE ocamlyacc HINTS "${OCAML_BINDIR}")

# generate ${CMAKE_CURRENT_BINARY_DIR}/lexer.ml
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lexer.ml
    COMMAND ${OCAMLLEX_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/lexer.mll -o lexer.ml
    MAIN_DEPENDENCY lexer.mll
)

# generate ${CMAKE_CURRENT_SOURCE_DIR}/oc_arith.ml{,i}
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/oc_arith.ml ${CMAKE_CURRENT_SOURCE_DIR}/oc_arith.mli
    COMMAND ${OCAMLYACC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/oc_arith.mly
    MAIN_DEPENDENCY oc_arith.mly
)

# generate ${CMAKE_CURRENT_BINARY_DIR}/arith.ml{,i}
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/arith.ml ${CMAKE_CURRENT_BINARY_DIR}/arith.mli
    COMMAND elkhound -ocaml -o arith ${CMAKE_CURRENT_SOURCE_DIR}/arith.gr
    DEPENDS elkhound arith.gr
)

# generate ${CMAKE_CURRENT_BINARY_DIR}/een.ml{,i}
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/een.ml ${CMAKE_CURRENT_BINARY_DIR}/een.mli
    COMMAND elkhound -ocaml -o een ${CMAKE_CURRENT_SOURCE_DIR}/een.gr
    DEPENDS elkhound een.gr
)

# ocaml_main files
# must be in this order!
set(OCAML_MAIN_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/parsetables.ml
    ${CMAKE_CURRENT_SOURCE_DIR}/useract.ml
    ${CMAKE_CURRENT_SOURCE_DIR}/smutil.ml
    ${CMAKE_CURRENT_SOURCE_DIR}/arraystack.ml
    ${CMAKE_CURRENT_SOURCE_DIR}/ptreenode.ml
    ${CMAKE_CURRENT_SOURCE_DIR}/lexerint.ml
    ${CMAKE_CURRENT_SOURCE_DIR}/ptreeact.ml

    # this is generated!
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.ml

    ${CMAKE_CURRENT_SOURCE_DIR}/lrparse.ml
    ${CMAKE_CURRENT_SOURCE_DIR}/objpool.ml

    # this is generated!
    ${CMAKE_CURRENT_SOURCE_DIR}/oc_arith.mli
    ${CMAKE_CURRENT_SOURCE_DIR}/oc_arith.ml

    # this is generated!
    ${CMAKE_CURRENT_BINARY_DIR}/arith.mli
    ${CMAKE_CURRENT_BINARY_DIR}/arith.ml

    # this is generated!
    ${CMAKE_CURRENT_BINARY_DIR}/een.mli
    ${CMAKE_CURRENT_BINARY_DIR}/een.ml

    ${CMAKE_CURRENT_SOURCE_DIR}/glr.ml
    ${CMAKE_CURRENT_SOURCE_DIR}/main.ml
)

# ocamlc flags
set(OCAMLC_FLAGS
    -I ${CMAKE_CURRENT_SOURCE_DIR}
    -I ${CMAKE_CURRENT_BINARY_DIR}
    -noassert
)

# Build ocaml_main
add_custom_target(ocaml_main ALL
    COMMAND ${OCAMLC_EXECUTABLE} -o ${CMAKE_CURRENT_BINARY_DIR}/ocaml_main -g ${OCAML_MAIN_FILES} ${OCAMLC_FLAGS}
    DEPENDS ${OCAML_MAIN_FILES}
)

# tests
if(BUILD_TESTING AND PERL_EXECUTABLE)
    add_test(
        NAME ocaml_main1
        COMMAND ${PERL_EXECUTABLE} ${SCRIPTS_DIR}/test-pipe "2+3" ${OCAMLRUN_EXECUTABLE} ocaml_main arith lr
    )
    add_test(
        NAME ocaml_main2
        COMMAND ${PERL_EXECUTABLE} ${SCRIPTS_DIR}/test-pipe "2+3" ${OCAMLRUN_EXECUTABLE} ocaml_main arith glr
    )
    add_test(
        NAME ocaml_main3
        COMMAND ${PERL_EXECUTABLE} ${SCRIPTS_DIR}/test-pipe "2+3" ${OCAMLRUN_EXECUTABLE} ocaml_main een glr
    )
    add_test(
        NAME ocaml_main4
        COMMAND ${PERL_EXECUTABLE} ${SCRIPTS_DIR}/test-pipe "2+3+4" ${OCAMLRUN_EXECUTABLE} ocaml_main een glr ptree
    )
elseif(NOT PERL_EXECUTABLE)
    message(WARNING " * Skipping the ocaml tests: Perl not found")
endif()
