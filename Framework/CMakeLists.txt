add_subdirectory(Backend)
set(BACKEND_PATH Backend)
set(TARGET framework)
set(FRAMEWORK_SRC
        ShaderCache.cpp
        ShaderCache.h
        VertexBuffer.cpp
        BufferBuilder.cpp
        IndexBuffer.cpp
        FixedPipeline.cpp
        BufferObject.cpp
        FixedPipeline.h IRenderProcessor.h VulkanRenderProcessor.cpp VulkanRenderProcessor.h IPipeline.h)




add_library(${TARGET} ${FRAMEWORK_SRC} ${BACKEND_SRC})

target_link_directories(${TARGET} PUBLIC ${VULKAN_LIB_PATH} ${SDL_LIB_PATH} ${SPDLOG_LIB_PATH})



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
set(SHADERC_INCLUDE_PATH
        ${THIRD_PARTY_ROOT}/shaderc/libshaderc/include
        ${THIRD_PARTY_ROOT}/shaderc/libshaderc_util/include)

list(APPEND ShaderLibs
        shaderc)

message("link library:" ${LINK_LIBS})
target_link_libraries(${TARGET} PRIVATE ${LINK_LIBS} VulkanMemoryAllocator glm utils ${ShaderLibs})
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${SHADERC_INCLUDE_PATH})