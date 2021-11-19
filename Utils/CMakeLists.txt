set(TARGET utils)

set(UTILS_SRC
        CircularBuffer.cpp
        Math/TransformUtils.cpp
        )

add_library(${TARGET} ${UTILS_SRC})