option(BUILD_WITH_TEX_BUILDER "tools to build useful tex" ON)

if (${BUILD_WITH_TEX_BUILDER})
    add_subdirectory(TexPreBuilder)
endif ()