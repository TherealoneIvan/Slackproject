
set(poco_headers
        ClassLibrary.h
        ClassLoader.h
        Exception.h
        Foundation.h
        Manifest.h
        MetaObject.h
        Mutex.h
        SharedLibrary.h
        )

set(poco_sources
        Manifest.cpp
        Exception.cpp
        SharedLibrary.cpp
        SharedLibrary_UNIX.cpp
        )

add_library(poco STATIC
        ${poco_sources}
        ${poco_headers}
        )

target_link_libraries(poco dl) 

