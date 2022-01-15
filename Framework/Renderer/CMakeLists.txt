set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(RenderGraph)

set(RENDERER_SRC
        ${RG_SRC}

        PARENT_SCOPE)