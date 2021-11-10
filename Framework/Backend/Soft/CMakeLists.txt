set(SOFT_RESOURCE_SRC
        ${SRC_PATH}/Soft/Resource/SoftRenderTarget.cpp)

set(SOFT_BACKEND_SRC
        ${SRC_PATH}/Soft/SoftDriver.cpp
        ${SRC_PATH}/Soft/SoftPipeline.cpp
        ${SRC_PATH}/Soft/SoftSwapChain.cpp

        ${SRC_PATH}/Soft/Utils/SoftTransform.cpp
        ${SOFT_RESOURCE_SRC}
        PARENT_SCOPE)