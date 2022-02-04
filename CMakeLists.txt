cmake_minimum_required(VERSION 3.20)
project(vulkan_eng
		VERSION 0.3
		DESCRIPTION "Vulkan based game engine"
		LANGUAGES CXX
		)

set(CMAKE_CXX_STANDARD 20)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB_RECURSE src_files
	 CONFIGURE_DEPENDS
	 src/*.cpp
	 )

#### TARGET ###
add_executable(vulkan_eng ${src_files} src/game/profiler.h)

target_include_directories(vulkan_eng PRIVATE src src/engine src/engine/renderer src/game)

####### LINK DEPENDENCY LIBS
# FetchContent added in CMake 3.11, downloads during the configure step
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED CACHE BOOL ON)

FetchContent_Declare(
		tinyobjloader
		GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git

)
FetchContent_Declare(
		glfw3
		GIT_REPOSITORY https://github.com/glfw/glfw.git
)
FetchContent_Declare(
		glm
		GIT_REPOSITORY https://github.com/g-truc/glm.git
)
FetchContent_Declare(
		reactphysics3d
		GIT_REPOSITORY https://github.com/DanielChappuis/reactphysics3d.git
		GIT_TAG v0.9.0
)
FetchContent_MakeAvailable(tinyobjloader glfw3 glm reactphysics3d)

##STBI IMAGE##
target_include_directories(vulkan_eng PRIVATE external/stbi)
######## reactPhysics ######
target_link_libraries(vulkan_eng PRIVATE reactphysics3d)
######## tiny #######
#set(tinyobjloader_DIR ${tinyobjloader_BINARY_DIR})
#find_package(tinyobjloader CONFIG REQUIRED)
target_link_libraries(vulkan_eng PRIVATE tinyobjloader)
##########glfw3#############
#find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(vulkan_eng PRIVATE glfw)
###########glm###############
#find_package(glm CONFIG REQUIRED)
target_link_libraries(vulkan_eng PRIVATE glm::glm)
###############vulkan###########
find_package(Vulkan REQUIRED)
target_link_libraries(vulkan_eng PRIVATE Vulkan::Vulkan)


## copy over models folder
add_custom_command(
		TARGET vulkan_eng POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/models
		${CMAKE_CURRENT_BINARY_DIR}/models
		COMMENT "copying models folder")




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
## clean out old shader
file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/shaders/)

##copy new shaders
file(COPY ${CMAKE_SOURCE_DIR}/shaders/
	 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders/)

##Copy shaders folder
add_custom_command(
		TARGET vulkan_eng PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E remove_directory
		${CMAKE_CURRENT_BINARY_DIR}/shaders/
		COMMENT "copying shaders folder")

##Copy shaders folder
add_custom_command(
		TARGET vulkan_eng POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/shaders/
		${CMAKE_CURRENT_BINARY_DIR}/shaders/
		COMMENT "copying shaders folder")