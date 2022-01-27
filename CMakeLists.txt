cmake_minimum_required(VERSION 3.20)
project(vulkan_eng
		VERSION 0.3
		DESCRIPTION "Vulkan based game engine"
		LANGUAGES CXX
		)

set(CMAKE_CXX_STANDARD 20)

# -DCMAKE_TOOLCHAIN_FILE=/usr/local/Cellar/vcpkg/2021.05.12/libexec/scripts/buildsystems/vcpkg.cmake
# -DCMAKE_TOOLCHAIN_FILE=[PATH TO VCPKG]/libexec/scripts/buildsystems/vcpkg.cmake

set_property(GLOBAL PROPERTY USE_FOLDERS ON)




# FetchContent added in CMake 3.11, downloads during the configure step
#include(FetchContent)
# FetchContent_MakeAvailable was not added until CMake 3.14; use our shim
#if (${CMAKE_VERSION} VERSION_LESS 3.14)
#	include(cmake/add_FetchContent_MakeAvailable.cmake)
#endif ()

##This should be the path to your own vcpkg
set(CMAKE_TOOLCHAIN_FILE: /usr/local/Cellar/vcpkg/2021.05.12/libexec/scripts/buildsystems/vcpkg.cmake)

file(GLOB_RECURSE src_files
	 CONFIGURE_DEPENDS
	 src/*.cpp
	 )


add_executable(vulkan_eng ${src_files})

target_include_directories(vulkan_eng PRIVATE src src/engine src/engine/renderer src/game)


## LINK DEPENDENCY LIBS
#tiny
find_package(tinyobjloader CONFIG REQUIRED)
target_link_libraries(vulkan_eng PRIVATE tinyobjloader::tinyobjloader)
#glfw3
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(vulkan_eng PRIVATE glfw)
#glm
find_package(glm CONFIG REQUIRED)
target_link_libraries(vulkan_eng PRIVATE glm::glm)
#vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(vulkan_eng PRIVATE Vulkan::Vulkan)


## copy over models folder
add_custom_command(
		TARGET vulkan_eng PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/models
		${CMAKE_CURRENT_BINARY_DIR}/models
		COMMENT "copying models folder")

##Copy shaders folder
add_custom_command(
		TARGET vulkan_eng POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/shaders/
		${CMAKE_CURRENT_BINARY_DIR}/shaders/
		COMMENT "copying shaders folder")

############## Build SHADERS #######################

# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
find_program(GLSL_VALIDATOR glslangValidator HINTS
			 ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
			 /usr/bin
			 /usr/local/bin
			 ${VULKAN_SDK_PATH}/Bin
			 ${VULKAN_SDK_PATH}/Bin32
			 $ENV{VULKAN_SDK}/Bin/
			 $ENV{VULKAN_SDK}/Bin32/
			 )

# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
	 "${PROJECT_SOURCE_DIR}/shaders/*.frag"
	 "${PROJECT_SOURCE_DIR}/shaders/*.vert"
	 )

foreach (GLSL ${GLSL_SOURCE_FILES})
	get_filename_component(FILE_NAME ${GLSL} NAME)
	set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
	add_custom_command(
			OUTPUT ${SPIRV}
			COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
			DEPENDS ${GLSL})
	list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)

add_custom_target(
		Shaders
		DEPENDS ${SPIRV_BINARY_FILES}
		COMMENT "shaders compiled"
)

