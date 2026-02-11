cmake_minimum_required(VERSION 3.10)
project(Melkam VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)


include_directories(include)
add_executable(Melkam
	src/main.cpp
	src/Melkam/core/engine.cpp
	src/Melkam/core/Logger.cpp
	src/Melkam/platform/Window.cpp
	 src/Melkam/platform/Input.cpp
	src/Melkam/core/Application.cpp
	src/Melkam/scene/Scene.cpp
	src/Melkam/scene/Entity.cpp
	src/Melkam/scene/Systems2D.cpp
	 src/Melkam/physics/Collider.cpp
	 src/Melkam/ui/Ui.cpp
)

set(RAYLIB_INCLUDE_DIR "C:/msys64/mingw64/include")
set(RAYLIB_LIBRARY_DIR "C:/msys64/mingw64/lib")

find_library(RAYLIB_LIBRARY raylib PATHS ${RAYLIB_LIBRARY_DIR} NO_DEFAULT_PATH)
if (NOT RAYLIB_LIBRARY)
	message(FATAL_ERROR "raylib library not found in ${RAYLIB_LIBRARY_DIR}")
endif()

include_directories(${RAYLIB_INCLUDE_DIR})
target_link_libraries(Melkam ${RAYLIB_LIBRARY})

set(RAYLIB_DLL "C:/msys64/mingw64/bin/raylib.dll")
if (EXISTS ${RAYLIB_DLL})
	add_custom_command(TARGET Melkam POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${RAYLIB_DLL}
			$<TARGET_FILE_DIR:Melkam>
	)
endif()

