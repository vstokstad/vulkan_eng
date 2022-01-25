#include "vs_simple_render_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


#include <cassert>
#include  <stdexcept>


struct simple_push_constant_data
{
	glm::mat4 transform{1.f};
	glm::mat4 normal_matrix{1.0f};
};

vs::vs_simple_render_system::vs_simple_render_system(vs_device& device, VkRenderPass render_pass) : device_(device)
{
	createPipelineLayout();
	createPipeline(render_pass);
}

vs::vs_simple_render_system::~vs_simple_render_system()
{
	vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
}

void vs::vs_simple_render_system::createPipelineLayout()
{
	VkPushConstantRange push_constant_range{};
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(simple_push_constant_data);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &push_constant_range;

	if (vkCreatePipelineLayout(device_.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout");
	}
}

void vs::vs_simple_render_system::createPipeline(VkRenderPass render_pass)
{
	assert(pipeline_layout_ != nullptr && "cannont create pipeline before pipeline layout");

	pipeline_config_info pipeline_config{};

	vs_pipeline::defaultPipelineConfigInfo(pipeline_config);
	pipeline_config.render_pass = render_pass;
	pipeline_config.pipeline_layout = pipeline_layout_;
	pipeline = std::make_unique<vs_pipeline>(
		device_,
		"shaders/simple_shader.vert.spv",
		"shaders/simple_shader.frag.spv",
		pipeline_config
	);
}


void vs::vs_simple_render_system::renderGameObjects(frame_info& frame_info,
                                                    std::vector<vs_game_object>& game_objects)
{
	pipeline->bind(frame_info.command_buffer);

	auto projection_view = frame_info.camera.getProjection() * frame_info.camera.getView();

	for (auto& object : game_objects)
	{
		simple_push_constant_data push{};


		push.transform = projection_view * object.transform.mat4();
		push.normal_matrix = object.transform.normal_matrix();

		vkCmdPushConstants(frame_info.command_buffer, pipeline_layout_,
		                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
		                   sizeof(simple_push_constant_data), &push);
		object.model->bind(frame_info.command_buffer);
		object.model->draw(frame_info.command_buffer);
	}
}
