#pragma once

#pragma once


#include <memory>
#include <vector>

#include "../renderer/vs_device.h"
#include "../vs_game_object.h"
#include "../renderer/vs_pipeline.h"
#include "../vs_frame_info.h"


namespace vs
{
	class vs_point_light_system
	{
	public:
		vs_point_light_system(vs_device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
		~vs_point_light_system();


		vs_point_light_system(const vs_point_light_system&) = delete;
		vs_point_light_system& operator==(const vs_point_light_system&) = delete;

		void render(frame_info& frame_info);


	private:
		void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
		void createPipeline(VkRenderPass render_pass);


		vs_device& device_;
		std::unique_ptr<vs_pipeline> pipeline;
		VkPipelineLayout pipeline_layout_;
	};
}
