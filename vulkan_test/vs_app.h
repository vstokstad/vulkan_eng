#pragma once

#include "vs_window.h"

namespace vs
{
	class vs_app
	{
	public:
		vs_app() = default;
		vs_app(int window_width_pixels);
		vs_app(float aspect_ratio, int window_width_pixels);
		vs_app(int window_width_pixels, int window_height_pixels);
	private:
		float aspect_ratio = 16.0f / 9.0f;
		int width = 720;
		int height = static_cast<int>(static_cast<float>(width) / aspect_ratio);
	public:
		void run() const;

	private:
		vs_window vs_window{ width, height, "Vulkan App" };
	};

}
