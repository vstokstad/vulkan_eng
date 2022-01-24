cmake_minimum_required(VERSION 3.19)
project(vulkan_eng)

set(CMAKE_CXX_STANDARD 20)
set(VULKAN_SDK /Users/vilhelmstokstad/VulkanSDK/1.2.198.1/macOs)
add_executable(vulkan_eng src/main.cpp src/vs_device.cpp src/vs_app.cpp src/vs_model.cpp src/vs_pipeline.cpp src/vs_swap_chain.cpp src/vs_window.cpp src/shaders/simple_shader.frag.spv src/shaders/simple_shader.vert.spv)

add_custom_command(
		TARGET vulkan_eng POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_SOURCE_DIR}/src/shaders/simple_shader.vert.spv
		${CMAKE_CURRENT_BINARY_DIR}/shaders/simple_shader.vert.spv)
add_custom_command(
		TARGET vulkan_eng POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_SOURCE_DIR}/src/shaders/simple_shader.frag.spv
		${CMAKE_CURRENT_BINARY_DIR}/shaders/simple_shader.frag.spv)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(vulkan_eng PRIVATE glfw)
find_package(glm CONFIG REQUIRED)
target_link_libraries(vulkan_eng PRIVATE glm::glm)
find_package(Vulkan REQUIRED)
target_link_libraries(vulkan_eng PRIVATE Vulkan::Vulkan)

