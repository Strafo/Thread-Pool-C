set(HEADER_FILES_LINKLIST
        atomic_defs.h
        linklist.h
        )
set(SOURCE_FILES_LINKLIST
        linklist.c
        )
add_library(LinkListLib  ${HEADER_FILES_LINKLIST} ${SOURCE_FILES_LINKLIST})
