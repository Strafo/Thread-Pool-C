find_package (Threads)
set(HEADER_FILES
        libhl/src/atomic_defs.h
        libhl/src/linklist.h
        threadpool.h
        )
set(SOURCE_FILES
        libhl/src/linklist.c
        threadpool.c
        )

add_library(ThreadPoolLib ${HEADER_FILES} ${SOURCE_FILES} )
target_link_libraries(ThreadPoolLib  ${CMAKE_THREAD_LIBS_INIT})