set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(SHADER_SRC
        ${ROOT_PATH}/Shader/CodeGenerator.cpp
        ${ROOT_PATH}/Shader/ShaderGenerator.cpp)

set(MATERIAL_SRC
        ${ROOT_PATH}/SamplerBlockGenerator.cpp
        ${ROOT_PATH}/UniformBlockGenerator.cpp
        ${ROOT_PATH}/MaterialParser.cpp
        ${ROOT_PATH}/SamplerBindingMap.cpp
        ${ROOT_PATH}/SamplerBlock.cpp
        ${ROOT_PATH}/ShaderBuilder.cpp
        ${ROOT_PATH}/ShaderCache.cpp
        ${ROOT_PATH}/UniformBlock.cpp
        ${ROOT_PATH}/UniformBuffer.cpp
        ${SHADER_SRC}
        PARENT_SCOPE)