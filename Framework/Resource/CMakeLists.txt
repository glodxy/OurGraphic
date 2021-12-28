set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Material)

set(FRAMEWORK_RESOURCE_SRC
        ${ROOT_PATH}/VertexBuffer.cpp
        ${ROOT_PATH}/BufferBuilder.cpp
        ${ROOT_PATH}/IndexBuffer.cpp
        ${ROOT_PATH}/BufferObject.cpp
        ${ROOT_PATH}/MaterialInstance.cpp
        ${ROOT_PATH}/MaterialCache.cpp
        ${MATERIAL_SRC}
        PARENT_SCOPE)

