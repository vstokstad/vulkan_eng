#include "vs_point_light_render_system.h"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

//std
#include <cassert>
#include  <stdexcept>

namespace vs
{
	struct point_light_push_constants
	{
		glm::vec4 position{}; // ignore w
		glm::vec4 color{}; // w is intensity
		float radius;
	};

        vs_point_light_render_system::vs_point_light_render_system(vs_device& device, VkRenderPass render_pass,
	                                             VkDescriptorSetLayout global_set_layout) : device_(device)
	{
		createPipelineLayout(global_set_layout);
		createPipeline(render_pass);
	}

        vs_point_light_render_system::~vs_point_light_render_system()
	{
		vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
	}

	void vs_point_light_render_system::createPipelineLayout(VkDescriptorSetLayout global_set_layout)
	{
		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(point_light_push_constants);

		std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptor_set_layouts.data();

		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &push_constant_range;

		if (vkCreatePipelineLayout(device_.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void
        vs_point_light_render_system::createPipeline(VkRenderPass render_pass)
	{
		assert(pipeline_layout_ != nullptr && "cannot create pipeline before pipeline layout");

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


	void vs_point_light_render_system::update(frame_info& frame_info, global_ubo& ubo)
	{
		auto rotate_light = glm::rotate(
			glm::mat4(1.f),
			(frame_info.frame_time * glm::half_pi<float>()),
			{0.1f, -1.f, .1f}
		);


		int light_index = 0;

		for (auto& kv : frame_info.lights)
		{
			auto& obj = kv.second;
			if (obj.point_light_comp ==nullptr) continue;


			assert(light_index <= MAX_LIGHTS && "reached max number of lights");
			//update position
			obj.transform_comp.translation = glm::vec3(
				rotate_light * glm::vec4(obj.transform_comp.translation, 1.f));


			//copy light to ubo
			ubo.point_lights[light_index].position = glm::vec4(obj.transform_comp.translation, 1.f);
			ubo.point_lights[light_index].color = glm::vec4(obj.color, obj.point_light_comp->light_intensity);

			light_index++;
		}
		ubo.num_lights = light_index;
	}

	void vs_point_light_render_system::render(frame_info& frame_info)
	{
		pipeline->bind(frame_info.command_buffer);

		vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1,
		                        &frame_info.global_descriptor_set, 0, nullptr);

		//check for lights and render
		for (auto& kv : frame_info.lights)
		{
			auto& obj = kv.second;
			if (obj.point_light_comp == nullptr) continue;
			point_light_push_constants push{};
			push.position = glm::vec4(obj.transform_comp.translation, 1.f);
			push.color = glm::vec4(obj.color, obj.point_light_comp->light_intensity);
			push.radius = obj.transform_comp.scale.x;

			vkCmdPushConstants(frame_info.command_buffer,
			                   pipeline_layout_,
			                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			                   0,
			                   sizeof(point_light_push_constants),
			                   &push
			);
			vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
		}
	}
}
