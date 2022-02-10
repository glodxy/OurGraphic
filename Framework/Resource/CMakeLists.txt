set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Material)
set(FRAMEWORK_RESOURCE_SRC
        ${MATERIAL_SRC}

        ${ROOT_PATH}/GlobalShaders.cpp
        ${ROOT_PATH}/VertexBuffer.cpp
        ${ROOT_PATH}/BufferBuilder.cpp
        ${ROOT_PATH}/IndexBuffer.cpp
        ${ROOT_PATH}/BufferObject.cpp
        ${ROOT_PATH}/MaterialInstance.cpp
        ${ROOT_PATH}/MaterialCache.cpp
        ${ROOT_PATH}/Material.cpp
        ${ROOT_PATH}/Texture.cpp
        ${ROOT_PATH}/RenderTarget.cpp
        ${ROOT_PATH}/MeshReader.cpp
        ${ROOT_PATH}/TextureLoader.cpp
        ${ROOT_PATH}/Skybox.cpp
        PARENT_SCOPE)

