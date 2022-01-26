#pragma once

#include "vs_window.h"
#include "renderer/vs_device.h"
#include "vs_game_object.h"
#include "renderer/vs_renderer.h"
#include "vs_descriptors.h"

//std
#include <memory>
#include <vector>

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

		//order of declaration matters (pool need to be constructed after device and destroyed before device.)
		std::unique_ptr<vs_descriptor_pool> global_descriptor_pool_{};
		vs_game_object::map game_objects_;
	};
}
