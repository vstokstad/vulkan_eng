#include "vs_window.h"

#include <iostream>

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
		glfwDestroyWindow(window_);
		glfwTerminate();
	}

	bool vs_window::shouldClose() const
	{
		return glfwWindowShouldClose(window_);
	}

	VkExtent2D vs_window::getExtent()
	{
		return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
	}

	void vs_window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window_, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window_ surface");
		}
	}

	void vs_window::frameBufferResizedCallback(GLFWwindow* window, int width, int height)
	{
		auto _window = reinterpret_cast<vs_window*>(glfwGetWindowUserPointer(window));
		_window->frame_buffer_resized_ = true;
		_window->width = width;
		_window->height = height;
	}


	auto vs_window::initWindow() -> int
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);


		window_ = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window_, this);
		glfwSetFramebufferSizeCallback(window_, frameBufferResizedCallback);
		if (!window_)
		{
                  throw std::runtime_error("failed to create window");
			return 1;
		}
		return 0;
	}
}
