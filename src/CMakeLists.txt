find_package (Threads)
add_subdirectory(libhl_linklist)
set(HEADER_FILES
        threadpool.h
        )
set(SOURCE_FILES
        threadpool.c
        )

add_library(ThreadPool STATIC ${HEADER_FILES} ${SOURCE_FILES} )
target_link_libraries(ThreadPool  ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(ThreadPool  LinkListLib)
