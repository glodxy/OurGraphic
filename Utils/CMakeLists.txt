set(TARGET utils)

set(UTILS_SRC
        CircularBuffer.cpp
        Math/TransformUtils.cpp
        Mesh/MeshImport.cpp
        Math/PackUtils.cpp
        HashUtils.h Container/AssociativeContainer.h
        Path/PathUtils.cpp

        Image/ImageEncoder.cpp
        Image/LinearImage.cpp)

add_library(${TARGET} STATIC ${UTILS_SRC})
target_link_libraries(${TARGET} PUBLIC ${ASSIMP_LIBS} glm png)