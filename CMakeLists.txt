cmake_minimum_required(VERSION 3.20)
project(MelkamEngine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(POLICY CMP0111)
    cmake_policy(SET CMP0111 OLD)
endif()


find_package(EnTT REQUIRED)
find_package(SDL3 REQUIRED)
find_package(box2d REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(VulkanMemoryAllocator REQUIRED)
find_package(volk REQUIRED)
find_package(Bullet REQUIRED)

add_executable(MelkamEngine
    src/main.cpp
    src/stb_impl.cpp
)

target_include_directories(MelkamEngine PUBLIC include include/helper)
target_link_libraries(MelkamEngine PRIVATE
    EnTT::EnTT
    SDL3::SDL3
    box2d::box2d
    vulkan-headers::vulkan-headers
    GPUOpen::VulkanMemoryAllocator
    volk::volk
    Bullet::Bullet
)

if(MSVC)
    target_compile_options(MelkamEngine PRIVATE /fp:fast /W3)
    target_compile_options(MelkamEngine PRIVATE $<$<CONFIG:Debug>:/Ob2>)
    target_compile_options(MelkamEngine PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot>)
else()
    target_compile_options(MelkamEngine PRIVATE -ffast-math -O3)
endif()

# Automatically bundle / copy project assets with the executable output
add_custom_command(TARGET MelkamEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:MelkamEngine>/assets
)

