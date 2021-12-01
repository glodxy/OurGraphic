set(SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(BACKEND_DRIVER_SRC)

if(${BUILD_WITH_VULKAN})
    message("USE VULKAN")
    add_subdirectory(Vulkan)
message(${VULKAN_BACKEND_SRC})
    list(APPEND
            BACKEND_DRIVER_SRC
            ${VULKAN_BACKEND_SRC})
endif()

if (${BUILD_WITH_SOFT})
    message("USE SOFT")
    add_subdirectory(Soft)
    list(APPEND
            BACKEND_DRIVER_SRC
            ${SOFT_BACKEND_SRC})
endif()

set(BACKEND_SRC
        ${BACKEND_SRC}
        ${SRC_PATH}/BaseRenderResource.cpp
        ${SRC_PATH}/BaseRenderResource.h
        ${SRC_PATH}/BaseRenderAction.cpp
        ${SRC_PATH}/BaseRenderAction.h
        ${SRC_PATH}/BackendUtils.cpp
        ${SRC_PATH}/Program.cpp
        ${SRC_PATH}/SamplerGroup.cpp
        ${SRC_PATH}/Commands.cpp
        ${SRC_PATH}/CommandBufferQueue.cpp
        ${SRC_PATH}/Driver.cpp
        ${BACKEND_DRIVER_SRC}
        PARENT_SCOPE)


