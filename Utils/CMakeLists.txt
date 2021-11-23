set(TARGET utils)

set(UTILS_SRC
        CircularBuffer.cpp
        Math/TransformUtils.cpp
        Mesh/MeshImport.cpp
        )

add_library(${TARGET} STATIC ${UTILS_SRC})
target_link_libraries(${TARGET} ${ASSIMP_LIBS})