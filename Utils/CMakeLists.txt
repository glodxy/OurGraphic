set(TARGET utils)

add_subdirectory(Image)

set(UTILS_SRC
        CircularBuffer.cpp
        Math/TransformUtils.cpp
        Mesh/MeshImport.cpp
        Math/PackUtils.cpp
        HashUtils.h Container/AssociativeContainer.h
        Path/PathUtils.cpp

        ${IMAGE_SRC})

add_library(${TARGET} STATIC ${UTILS_SRC})
target_link_libraries(${TARGET} PUBLIC ${ASSIMP_LIBS} glm png)