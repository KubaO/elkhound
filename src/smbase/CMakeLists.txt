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
    hashtbl.cc
    nonport.cpp
    point.cc
    srcloc.cc
    str.cpp
    strhash.cc
    strtokp.cpp
    strutil.cc
    svdict.cc
    syserr.cpp
    trace.cc
    trdelete.cc
    vdtllist.cc
    voidlist.cc
)
