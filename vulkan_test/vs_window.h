#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace vs
{
	class vs_window
	{
	public:
		explicit vs_window(int w, int h, std::string name);
		~vs_window();

		//remove non-used constructors to avoid creating copies of the main window.
		vs_window(const vs_window&) = delete;
		vs_window& operator=(const vs_window&) = delete;

		/**
		 * \brief Is this window waiting to be closed?
		 * \return true if the window has had a should_close event queried else false.
		 */
		bool shouldClose() const;

		VkExtent2D getExtent();

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		int width, height;
		std::string window_name;
		GLFWwindow* window;

		int initWindow();
	};
}
