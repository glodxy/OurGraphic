cmake_minimum_required(VERSION 3.17)
if (WIN32)
    set(CMAKE_CXX_STANDARD 20)
elseif(APPLE)
    set(CMAKE_CXX_STANDARD 17)
endif ()

project(our_graphic)

set(CMAKE_BUILD_TYPE Debug)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DDEBUG)
    set(DEBUG true)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
    add_definitions(-DRELEASE)
    set(RELEASE true)
endif()

set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

option(BUILD_WITH_VULKAN "if on, vulkan will be added to source" OFF)
option(BUILD_WITH_SOFT "if on, soft driver will be added to source" ON)

set(VULKAN_INCLUDE_PATH ${ROOT_PATH}/ThirdParty/Vulkan/include)
set(SDL_INCLUDE_PATH ${ROOT_PATH}/ThirdParty/SDL2/include)
set(SPDLOG_INCLUDE_PATH ${ROOT_PATH}/ThirdParty/spdlog/include)
set(VMA_INCLUDE_PATH ${ROOT_PATH}/ThirdParty/VulkanMemoryAllocator/include)

if(WIN32)
    ADD_DEFINITIONS(-DSDL_VIDEO_DRIVER_WINDOWS)
    set(VULKAN_LIB_PATH ${ROOT_PATH}/ThirdParty/Vulkan/lib/windows)
    set(VULKAN_LIBS vulkan-1 VKLayer_utils)

    set(SDL_LIB_PATH ${ROOT_PATH}/ThirdParty/SDL2/lib/windows)
    set(SDL_LIBS SDL2 SDL2main)
elseif(APPLE)
    ADD_DEFINITIONS(-DSDL_VIDEO_DRIVER_COCOA)
    message(APPLE libs)
    set(VULKAN_LIB_PATH ${ROOT_PATH}/ThirdParty/Vulkan/lib/macos)
    set(VULKAN_LIBS libvulkan.dylib libvulkan.1.dylib)

    set(SDL_LIB_PATH ${ROOT_PATH}/ThirdParty/SDL2/lib/macos)
    set(SDL_LIBS libSDL2-2.0.dylib)
endif()
set(Vulkan_LIBRARY ${VULKAN_LIBS})
if(WIN32)
    set(SPDLOG_LIB_PATH ${ROOT_PATH}/ThirdParty/spdlog/lib/windows)
    if (DEBUG)
        set(SPDLOG_LIBS spdlogd)
    else()
        set(SPDLOG_LIBS spdlog)
    endif()
elseif(APPLE)
    set(SPDLOG_LIB_PATH ${ROOT_PATH}/ThirdParty/spdlog/lib/macos)
    set(SPDLOG_LIBS spdlog)
endif()
set(THIRD_PARTY_ROOT ThirdParty)
include_directories(${VULKAN_INCLUDE_PATH}
                    ${SDL_INCLUDE_PATH}
                    ${SPDLOG_INCLUDE_PATH}
                    ${VMA_INCLUDE_PATH})
include_directories(ThirdParty)
include_directories(ThirdParty/assimp/include)
include_directories(${CMAKE_SOURCE_DIR})
if (WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/debug)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/release)
else()
    set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_SOURCE_DIR}/bin)
endif ()

if (TEST)
    set(MAIN_SRC)
    add_subdirectory(test)
    set(LINK_LIBS glm ${VULKAN_LIBS} ${SDL_LIBS} ${SPDLOG_LIBS})
else()
    set(MAIN_SRC main.cpp)
    set(BACKEND_SRC)
    set(LINK_LIBS glm
            Assimp
            ${SDL_LIBS}
            ${SPDLOG_LIBS})
endif()




if (${BUILD_WITH_VULKAN})
    add_definitions(-DBUILD_WITH_VULKAN)
    add_subdirectory(ThirdParty/VulkanMemoryAllocator)
    set(SHADERC_SKIP_TESTS true)
    set(SHADERC_SKIP_EXAMPLES true)
    add_subdirectory(ThirdParty/shaderc)

    list(APPEND LINK_LIBS
            ${VULKAN_LIBS})
endif ()
if (${BUILD_WITH_SOFT})
    add_definitions(-DBUILD_WITH_SOFT)
endif ()

add_subdirectory(ThirdParty/assimp)
add_subdirectory(ThirdParty/glm)
add_subdirectory(Utils)
add_subdirectory(Framework)

add_executable(our_graphic ${MAIN_SRC})

set(BUILD_STATIC_LIBS)


target_link_libraries(our_graphic framework)
#target_link_directories(our_graphic PUBLIC ${VULKAN_LIB_PATH} ${SDL_LIB_PATH} ${SPDLOG_LIB_PATH})
#
#if(WIN32)
#    target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})
#elseif(APPLE)
#    target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})
#endif()
if (WIN32)
    message(use msvc)
    if (DEBUG)
        file(COPY ${SDL_LIB_PATH}/SDL2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
    else()
        file(COPY ${SDL_LIB_PATH}/SDL2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
    endif()
elseif(APPLE)
    message(${EXECUTABLE_OUTPUT_PATH})
    message(apple)
    file(GLOB ${VULKAN_LIBB_PATH}/*.dylib SHARE_LIBS)
    file(COPY ${VULKAN_LIB_PATH}/${SHARE_LIBS} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
    file(COPY ${SDL_LIB_PATH}/libSDL2-2.0.dylib DESTINATION ${EXECUTABLE_OUTPUT_PATH})
else()
    file(COPY ${SDL_LIB_PATH}/SDL2.dll DESTINATION ${EXECUTABLE_OUTPUT_PATH})
endif ()
