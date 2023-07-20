#
# ast CMakeLists.txt
#
project(smbase)

# all the files
add_library(smbase STATIC
    autofile.cc
    bflatten.cc
    bit2d.cc
    breaker.cpp
    crc.cpp
    cycles.c
    exc.cpp
    flatten.cc
    gprintf.c
    hashline.cc
    nonport.cpp
    point.cc
    srcloc.cc
    str.cpp
    strtokp.cpp
    strutil.cc
    syserr.cpp
    trace.cc
    trdelete.cc
)

target_link_libraries(smbase PUBLIC fmt::fmt)
