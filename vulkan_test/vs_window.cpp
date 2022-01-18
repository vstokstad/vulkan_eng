#include "vs_window.h"

#include <iostream>
#include <utility>

namespace vs {

	vs_window::vs_window(const int w, const int h, std::string name) : width(w), height(h), window_name(std::move(name))
	{
		init_window();
	}
	vs_window::~vs_window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	auto vs_window::should_close() const -> bool
	{
		return	glfwWindowShouldClose(window);
	}

	auto vs_window::init_window() -> int
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
		if (!window)
		{
			printf_s("failed creating window");
			return 1;

		}
		return 0;
	}
}
