#pragma once


#include <memory>
#include <vector>

#include "vs_device.h"
#include "vs_game_object.h"
#include "vs_pipeline.h"
#include "vs_camera.h"


namespace vs
{
	class vs_simple_render_system
	{
	public:
		vs_simple_render_system(vs_device& device, VkRenderPass render_pass);
		~vs_simple_render_system();


		vs_simple_render_system(const vs_simple_render_system&) = delete;
		vs_simple_render_system& operator==(const vs_simple_render_system&) = delete;

		void renderGameObjects(VkCommandBuffer command_buffer, std::vector<vs_game_object>& game_objects,
		                       const vs_camera& camera);


	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass render_pass);


		vs_device& device_;
		std::unique_ptr<vs_pipeline> pipeline;
		VkPipelineLayout pipeline_layout_;
	};
}
