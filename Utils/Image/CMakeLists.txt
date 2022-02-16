set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(IMAGE_SRC
        ${ROOT_PATH}/ImageEncoder.cpp
        ${ROOT_PATH}/LinearImage.cpp
        ${ROOT_PATH}/Cubemap/Cubemap.cpp
        ${ROOT_PATH}/Cubemap/CubemapIBL.cpp
        ${ROOT_PATH}/Cubemap/CubemapUtils.cpp

        PARENT_SCOPE)