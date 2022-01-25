#pragma once

#include <memory>
#include <vector>

#include "vs_window.h"
#include "vs_device.h"


#include "vs_game_object.h"
#include "vs_renderer.h"

namespace vs
{
	class vs_model;

	class vs_app
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		vs_app();
		~vs_app();


		vs_app(const vs_app&) = delete;
		vs_app& operator==(const vs_app&) = delete;


		void run();


	private:
		void loadGameObjects();


		vs_window window_{WIDTH, HEIGHT, "Vulkan App"};
		vs_device device_{window_};
		vs_renderer renderer_{window_, device_};

		std::vector<vs_game_object> game_objects_;
	};
}
