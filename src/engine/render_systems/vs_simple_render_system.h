#pragma once


#include <memory>

#include "engine/renderer/vs_device.h"
#include "engine/renderer/vs_pipeline.h"
#include "engine/vs_frame_info.h"


namespace vs
{
	class vs_simple_render_system
	{
	public:
		vs_simple_render_system(vs_device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
		~vs_simple_render_system();


		vs_simple_render_system(const vs_simple_render_system&) = delete;
		vs_simple_render_system& operator==(const vs_simple_render_system&) = delete;

		void renderGameObjects(frame_info& frame_info);


	private:
		void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
		void createPipeline(VkRenderPass render_pass);


		vs_device& device_;
		std::unique_ptr<vs_pipeline> pipeline;
		VkPipelineLayout pipeline_layout_;
	};
}
