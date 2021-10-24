set(TARGET utils)

set(UTILS_SRC
        CircularBuffer.cpp
        )

add_library(${TARGET} ${UTILS_SRC})