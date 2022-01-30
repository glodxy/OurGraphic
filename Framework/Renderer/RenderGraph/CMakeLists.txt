set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(RG_BASE_SRC
        ${ROOT_PATH}/Base/DependencyGraph.cpp
        ${ROOT_PATH}/Base/PassNode.cpp
        ${ROOT_PATH}/Base/Resource.cpp
        ${ROOT_PATH}/Base/ResourceNode.cpp)

set(RG_PASS_SRC
        ${ROOT_PATH}/Pass/RenderGraphPass.cpp)

set(RG_RESOURCE_SRC
        ${ROOT_PATH}/Resource/RenderGraphResources.cpp
        ${ROOT_PATH}/Resource/RenderGraphTexture.cpp)

set(RG_SRC
        ${RG_BASE_SRC}
        ${RG_PASS_SRC}
        ${RG_RESOURCE_SRC}
        ${ROOT_PATH}/RenderGraph.cpp
        ${ROOT_PATH}/RenderGraphResourceAllocator.cpp
        PARENT_SCOPE)