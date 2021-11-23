add_subdirectory(Backend)
set(BACKEND_PATH Backend)
set(TARGET framework)
set(FRAMEWORK_RESOURCE_SRC
        Resource/VertexBuffer.cpp
        Resource/BufferBuilder.cpp
        Resource/IndexBuffer.cpp
        Resource/BufferObject.cpp)

set(FRAMEWORK_COMPONENT_SRC
        Component/Camera.cpp
        Component/Transform.cpp)

set(FRAMEWORK_SYSTEM_SRC
        Manager/Entity.cpp
        Manager/EntityManager.cpp
        Manager/SystemManager.cpp
        Manager/MeshReader.cpp
        Manager/RenderSystem.cpp)

set(FRAMEWORK_SRC
        FixedPipeline.cpp
        FixedPipeline.h IRenderProcessor.h
        SoftRenderProcessor.cpp
        ${FRAMEWORK_RESOURCE_SRC}
        ${FRAMEWORK_COMPONENT_SRC}
        ${FRAMEWORK_SYSTEM_SRC}
        )

if (${BUILD_WITH_VULKAN})
    list(APPEND
            FRAMEWORK_SRC
            Resource/ShaderCache.cpp
            Resource/ShaderCache.h
            VulkanRenderProcessor.cpp VulkanRenderProcessor.h)
endif ()

add_library(${TARGET} STATIC ${FRAMEWORK_SRC} ${BACKEND_SRC})



set(FRAMEWORK_INCLUDE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR})
set(FRAMEWORK_LINK_PATH
        ${SDL_LIB_PATH}
        ${SPDLOG_LIB_PATH})
set(FRAMEWORK_LINK_LIBS
        ${LINK_LIBS}
        glm
        utils)

if (${BUILD_WITH_VULKAN})
    set(SHADERC_INCLUDE_PATH
            ${THIRD_PARTY_ROOT}/shaderc/libshaderc/include
            ${THIRD_PARTY_ROOT}/shaderc/libshaderc_util/include)

    set(SHADERC_LIBS
            shaderc)

    list(APPEND FRAMEWORK_LINK_PATH
            ${VULKAN_LIB_PATH})

    list(APPEND FRAMEWORK_LINK_LIBS
            ${SHADERC_LIBS}
            VulkanMemoryAllocator)

    list(APPEND FRAMEWORK_INCLUDE_PATH
            ${SHADERC_INCLUDE_PATH})
endif ()



if (APPLE)
    set(OPTIMIZATION_FLAGS
            -ffast-math
            -ffp-contract=fast
            # TODO: aggressive vectorization is currently broken on Android
            #        -fslp-vectorize-aggressive
            -fvisibility-inlines-hidden
            -fstrict-vtable-pointers
            )
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")

#
    target_link_libraries(${TARGET} PRIVATE "-framework Cocoa" "-framework QuartzCore")
    target_compile_options(${TARGET} PRIVATE "-fobjc-arc")
endif ()


message("link library:" ${LINK_LIBS})
target_link_directories(${TARGET} PUBLIC ${FRAMEWORK_LINK_PATH})
target_link_libraries(${TARGET} PRIVATE ${FRAMEWORK_LINK_LIBS})
target_include_directories(${TARGET} PUBLIC ${FRAMEWORK_INCLUDE_PATH})