#include "vs_point_light_system.h"

#include <cassert>
#include  <stdexcept>

namespace vs
{
	vs_point_light_system::vs_point_light_system(vs_device& device, VkRenderPass render_pass,
	                                             VkDescriptorSetLayout global_set_layout) : device_(device)
	{
		createPipelineLayout(global_set_layout);
		createPipeline(render_pass);
	}

	vs_point_light_system::~vs_point_light_system()
	{
		vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
	}

	void vs_point_light_system::createPipelineLayout(VkDescriptorSetLayout global_set_layout)
	{
		// VkPushConstantRange push_constant_range{};
		// push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		// push_constant_range.offset = 0;
		// push_constant_range.size = sizeof(simple_push_constant_data);

		std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptor_set_layouts.data();

		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(device_.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void vs_point_light_system::createPipeline(VkRenderPass render_pass)
	{
		assert(pipeline_layout_ != nullptr && "cannont create pipeline before pipeline layout");

		pipeline_config_info pipeline_config{};

		vs_pipeline::defaultPipelineConfigInfo(pipeline_config);
		pipeline_config.attribute_descriptions.clear();
		pipeline_config.binding_descriptions.clear();
		pipeline_config.render_pass = render_pass;
		pipeline_config.pipeline_layout = pipeline_layout_;
		pipeline = std::make_unique<vs_pipeline>(
			device_,
			"shaders/point_light.vert.spv",
			"shaders/point_light.frag.spv",
			pipeline_config
		);
	}


	void vs_point_light_system::render(frame_info& frame_info)
	{
		pipeline->bind(frame_info.command_buffer);

		vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1,
		                        &frame_info.global_descriptor_set, 0, nullptr);

		vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
	}
}
