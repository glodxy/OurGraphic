set(SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(VULKAN_BACKEND_SRC
        ${SRC_PATH}/Vulkan/VulkanInstance.cpp
        ${SRC_PATH}/Vulkan/VulkanInstance.h
        ${SRC_PATH}/Vulkan/VulkanDevice.cpp
        ${SRC_PATH}/Vulkan/VulkanDevice.h
        ${SRC_PATH}/Vulkan/VulkanCommandBuffer.cpp
        ${SRC_PATH}/Vulkan/VulkanCommandBuffer.h
        ${SRC_PATH}/Vulkan/VulkanCommandPool.cpp
        ${SRC_PATH}/Vulkan/VulkanCommandPool.h
        ${SRC_PATH}/Vulkan/VulkanMemoryAllocator.cpp
        ${SRC_PATH}/Vulkan/VulkanMemoryAllocator.h
        ${SRC_PATH}/Vulkan/VulkanMemoryHandle.cpp
        ${SRC_PATH}/Vulkan/VulkanMemoryHandle.h
        ${SRC_PATH}/Vulkan/VulkanTexture.cpp
        ${SRC_PATH}/Vulkan/VulkanTexture.h
        ${SRC_PATH}/Vulkan/VulkanTextureBuffer.cpp
        ${SRC_PATH}/Vulkan/VulkanTextureBuffer.h
        ${SRC_PATH}/Vulkan/VulkanTextureView.cpp
        ${SRC_PATH}/Vulkan/VulkanTextureView.h
        ${SRC_PATH}/Vulkan/VulkanBaseBuffer.cpp
        ${SRC_PATH}/Vulkan/VulkanBaseBuffer.h
        ${SRC_PATH}/Vulkan/VulkanBaseBufferView.cpp
        ${SRC_PATH}/Vulkan/VulkanBaseBufferView.h
        ${SRC_PATH}/Vulkan/VulkanBuffer.cpp
        ${SRC_PATH}/Vulkan/VulkanBuffer.h
        ${SRC_PATH}/Vulkan/VulkanSwapChain.cpp
        ${SRC_PATH}/Vulkan/VulkanSwapChain.h
        ${SRC_PATH}/Vulkan/VulkanUtils.cpp
        ${SRC_PATH}/Vulkan/VulkanFBOCache.cpp
        ${SRC_PATH}/Vulkan/VulkanFBOCache.h)

if(APPLE)
    set(VULKAN_BACKEND_SRC
            ${VULKAN_BACKEND_SRC}
            ${SRC_PATH}/Vulkan/VulkanPlatformMacos.cpp
            ${SRC_PATH}/Vulkan/VulkanPlatformMacos.h)
elseif(WIN32)
    set(VULKAN_BACKEND_SRC
            ${VULKAN_BACKEND_SRC}
            ${SRC_PATH}/Vulkan/VulkanPlatformWindows.cpp
            ${SRC_PATH}/Vulkan/VulkanPlatformWindows.h)
endif()

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
