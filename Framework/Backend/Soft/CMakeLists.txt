set(SOFT_RESOURCE_SRC
        ${SRC_PATH}/Soft/Resource/SoftRenderTarget.cpp
        ${SRC_PATH}/Soft/Resource/SoftRenderPrimitive.cpp
        ${SRC_PATH}/Soft/Resource/SoftBuffer.cpp)

set(SOFT_SHADER_SRC
        ${SRC_PATH}/Soft/Shader/SimpleBlinnPhongShade.cpp)

set(SOFT_BACKEND_SRC
        ${SRC_PATH}/Soft/SoftDriver.cpp
        ${SRC_PATH}/Soft/SoftPipeline.cpp
        ${SRC_PATH}/Soft/SoftSwapChain.cpp
        ${SRC_PATH}/Soft/SimpleSoftRasterizer.cpp
        ${SRC_PATH}/Soft/SimpleSoftTest.cpp
        ${SRC_PATH}/Soft/Utils/SoftTransform.cpp
        ${SOFT_RESOURCE_SRC}
        ${SOFT_SHADER_SRC}
        PARENT_SCOPE)