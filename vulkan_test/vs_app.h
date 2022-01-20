#pragma once

#include <memory>
#include <vector>
#include <glm/vec2.hpp>

#include "vs_pipeline.h"
#include "vs_window.h"
#include "vs_device.h"
#include "vs_model.h"
#include "vs_swap_chain.h"

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
		void sierpinski(
			std::vector<vs_model::vertex>& vertices,
			int depth,
			glm::vec2 left,
			glm::vec2 right,
			glm::vec2 top,
			glm::vec3 color);
		void loadModels();

	private:
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();

		vs_window window_{WIDTH, HEIGHT, "Vulkan App"};
		vs_device device_{window_};
		vs_swap_chain swap_chain_{device_, window_.getExtent()};
		std::unique_ptr<vs_pipeline> pipeline;
		std::unique_ptr<vs_model> model_;
		VkPipelineLayout pipeline_layout_;
		std::vector<VkCommandBuffer> command_buffers_;
	};
}
