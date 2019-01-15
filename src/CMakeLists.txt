
set(HEADERS_FILES
        atomic_defs.h
        linklist.h
        threadpool.h
        )
set(SOURCE_FILES
        linklist.c
        threadpool.c
        )
add_library(Thread-Pool-C-lib  ${SOURCE_FILES} ${HEADERS_FILES})
set_target_properties(Thread-Pool-C-lib PROPERTIES
        IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/src"
        INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/src/threadpool.h"
        )
















