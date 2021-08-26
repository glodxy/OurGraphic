add_subdirectory(Backend)
set(BACKEND_PATH Backend)

set(FRAMEWORK_SRC
        FixedPipeline.cpp
        FixedPipeline.h)

add_library(framework ${FRAMEWORK_SRC} ${BACKEND_SRC})