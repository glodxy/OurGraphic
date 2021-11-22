set(TARGET utils)

set(UTILS_SRC
        CircularBuffer.cpp
        Math/TransformUtils.cpp
        Mesh/MeshImport.cpp
        )

add_library(${TARGET} ${UTILS_SRC})
target_link_libraries(${TARGET} Assimp.dll)