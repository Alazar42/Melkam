cmake_minimum_required(VERSION 3.20)
project(MelkamEngine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(POLICY CMP0111)
    cmake_policy(SET CMP0111 OLD)
endif()


find_package(EnTT REQUIRED)
find_package(SDL3 REQUIRED)
find_package(box2d REQUIRED)

add_executable(MelkamEngine
    src/main.cpp
    src/stb_impl.cpp
)

target_include_directories(MelkamEngine PUBLIC include include/helper)
target_link_libraries(MelkamEngine PRIVATE EnTT::EnTT SDL3::SDL3 box2d::box2d)

# Automatically bundle / copy project assets with the executable output
add_custom_command(TARGET MelkamEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:MelkamEngine>/assets
)

