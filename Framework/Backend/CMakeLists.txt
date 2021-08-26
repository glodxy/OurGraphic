set(SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(BACKEND_SRC
        ${BACKEND_SRC}
        ${SRC_PATH}/BaseRenderResource.cpp
        ${SRC_PATH}/BaseRenderResource.h
        ${SRC_PATH}/BaseRenderAction.cpp
        ${SRC_PATH}/BaseRenderAction.h
        PARENT_SCOPE)

if(BUILD_WITH_VULKAN)
    add_subdirectory(Vulkan)
endif()
