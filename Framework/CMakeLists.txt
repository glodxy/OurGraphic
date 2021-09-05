add_subdirectory(Backend)
set(BACKEND_PATH Backend)

set(FRAMEWORK_SRC
        FixedPipeline.cpp
        FixedPipeline.h IRenderProcessor.h VulkanRenderProcessor.cpp VulkanRenderProcessor.h IPipeline.h)


add_library(framework ${FRAMEWORK_SRC} ${BACKEND_SRC})
target_include_directories(framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})