cmake_minimum_required(VERSION 3.15)
project(MelkamEngine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(POLICY CMP0111)
    cmake_policy(SET CMP0111 OLD)
endif()

find_package(EnTT REQUIRED)
find_package(SDL3 REQUIRED)

add_executable(MelkamEngine
    src/main.cpp
    include/ECS.hpp
    include/Window.hpp
    include/input.hpp
    include/time.hpp
    include/nodes/2D/Camera2D.hpp
    include/renderers/Renderer2D.hpp
)

target_include_directories(MelkamEngine PUBLIC include include/helper)
target_link_libraries(MelkamEngine PRIVATE EnTT::EnTT SDL3::SDL3)
