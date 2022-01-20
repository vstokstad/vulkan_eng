#include "vs_window.h"

#include <iostream>
#include <utility>

namespace vs
{
	vs_window::vs_window(const int w, const int h, std::string name) : width(w),
	                                                                   height(h),
	                                                                   window_name(name)
	{
		initWindow();
	}

	vs_window::~vs_window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	bool vs_window::shouldClose() const
	{
		return glfwWindowShouldClose(window);
	}

	VkExtent2D vs_window::getExtent()
	{
		return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
	}

	void vs_window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface");
		}
	}

	auto vs_window::initWindow() -> int
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
