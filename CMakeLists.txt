cmake_minimum_required(VERSION 3.20)
project(our_graphic)
set(VULKAN_INCLUDE_PATH ThirdParty/Vulkan/include)
set(SDL_INCLUDE_PATH ThirdParty/SDL2/include)
set(MAIN_SRC main.cpp)

include_directories(${VULKAN_INCLUDE_PATH}
                    ${SDL_INCLUDE_PATH})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

add_executable(our_graphic ${MAIN_SRC})

set(VULKAN_LIB_PATH ThirdParty/Vulkan/lib)
set(VULKAN_LIBS vulkan-1 VKLayer_utils)

set(SDL_LIB_PATH ThirdParty/SDL2/lib)
set(SDL_LIBS SDL2 SDL2main)

set(BUILD_STATIC_LIBS)
add_subdirectory(ThirdParty/glm)

target_link_directories(our_graphic PUBLIC ${VULKAN_LIB_PATH} ${SDL_LIB_PATH})

target_link_libraries(${PROJECT_NAME} glm
                        ${VULKAN_LIBS}
                        ${SDL_LIBS})

file(COPY ${SDL_LIB_PATH}/SDL2.dll DESTINATION ${EXECUTABLE_OUTPUT_PATH})