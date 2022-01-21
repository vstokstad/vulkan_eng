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

		//remove non-used constructors to avoid creating copies of the main window_.
		vs_window(const vs_window&) = delete;
		vs_window& operator=(const vs_window&) = delete;

		/**
		 * \brief Is this window waiting to be closed?
		 * \return true if the window has had a should_close event queried else false.
		 */
		bool shouldClose() const;


		VkExtent2D getExtent();
		bool wasFrameBufferResized() { return frame_buffer_resized_; }
		void resetFrameBufferResizedFlag() { frame_buffer_resized_ = false; }


		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		static void frameBufferResizedCallback(GLFWwindow* window, int width, int height);
		int width, height;
		bool frame_buffer_resized_ = false;
		std::string window_name;
		GLFWwindow* window_;

		int initWindow();
	};
}
