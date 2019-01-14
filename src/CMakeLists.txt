
set(HEADERS_FILES
        atomic_defs.h
        linklist.h
        threadpool.h
        )
set(SOURCE_FILES
        linklist.c
        threadpool.c
        )
add_library(ThreadPoolCLib  ${SOURCE_FILES} ${HEADERS_FILES})

