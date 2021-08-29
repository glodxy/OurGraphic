set(SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(VULKAN_BACKEND_SRC
        ${SRC_PATH}/Vulkan/VulkanInstance.cpp
        ${SRC_PATH}/Vulkan/VulkanInstance.h
        ${SRC_PATH}/Vulkan/VulkanDevice.cpp
        ${SRC_PATH}/Vulkan/VulkanDevice.h
        ${SRC_PATH}/Vulkan/VulkanCommandBuffer.cpp
        ${SRC_PATH}/Vulkan/VulkanCommandBuffer.h)

set(BACKEND_SRC
        ${BACKEND_SRC}
        ${SRC_PATH}/BaseRenderResource.cpp
        ${SRC_PATH}/BaseRenderResource.h
        ${SRC_PATH}/BaseRenderAction.cpp
        ${SRC_PATH}/BaseRenderAction.h

        ${VULKAN_BACKEND_SRC}
        PARENT_SCOPE)

if(BUILD_WITH_VULKAN)
    add_subdirectory(Vulkan)
endif()
