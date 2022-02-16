set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(TARGET TexBuilder)

set(SRC
        ${ROOT_PATH}/main.cpp)

add_executable(${TARGET} ${SRC})
target_link_libraries(${TARGET} utils)