#include "vs_pipeline.h"

#include <fstream>
#include <cassert>
#include <iostream>
#include <stdexcept>

#include "../vs_model.h"

namespace vs
{
	vs_pipeline::vs_pipeline(vs_device& device,
	                         const std::string& vert_path,
	                         const std::string& frag_path,
	                         const pipeline_config_info& config)
		: device_{device}
	{
		create_graphics_pipeline(vert_path, frag_path, config);
	}

	vs_pipeline::~vs_pipeline()
	{
		vkDestroyShaderModule(device_.device(), vert_shader_module_, nullptr);
		vkDestroyShaderModule(device_.device(), frag_shader_module_, nullptr);
		vkDestroyPipeline(device_.device(), graphics_pipeline_, nullptr);
	}

	void vs_pipeline::bind(VkCommandBuffer command_buffer)
	{
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);
	}


	std::vector<char> vs_pipeline::readFile(const std::string& filepath)
	{
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file: " + filepath);
		}
		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}

	void vs_pipeline::create_graphics_pipeline(const std::string& vert_path,
	                                           const std::string& frag_path,
	                                           const pipeline_config_info& config_info)
	{
		assert(
			config_info.pipeline_layout != VK_NULL_HANDLE &&
			"Cannot create graphics pipeline:: no pipelineLayout provided in config_info");
		assert(
			config_info.render_pass != VK_NULL_HANDLE &&
			"Cannot create graphics pipeline:: no renderPass provided in config_info");

		auto vert_code = readFile(vert_path);
		auto frag_code = readFile(frag_path);


		create_shader_module(vert_code, &vert_shader_module_);
		create_shader_module(frag_code, &frag_shader_module_);

		VkPipelineShaderStageCreateInfo shader_stages[2]{};
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader_stages[0].module = vert_shader_module_;
		shader_stages[0].pName = "main";
		shader_stages[0].flags = 0;
		shader_stages[0].pNext = nullptr;
		shader_stages[0].pSpecializationInfo = nullptr;

		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_stages[1].module = frag_shader_module_;
		shader_stages[1].pName = "main";
		shader_stages[1].flags = 0;
		shader_stages[1].pNext = nullptr;
		shader_stages[1].pSpecializationInfo = nullptr;

		auto bindingDescription = vs_model::vertex::getBindingDescriptions();
		auto attributeDescription = vs_model::vertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertex_input_info{};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
		vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
		vertex_input_info.pVertexAttributeDescriptions = attributeDescription.data();
		vertex_input_info.pVertexBindingDescriptions = bindingDescription.data();


		VkGraphicsPipelineCreateInfo pipeline_info{};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_stages;
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &config_info.input_assembly_info;
		pipeline_info.pViewportState = &config_info.viewport_info;
		pipeline_info.pRasterizationState = &config_info.rasterization_info;
		pipeline_info.pMultisampleState = &config_info.multisample_info;
		pipeline_info.pColorBlendState = &config_info.color_blend_info;
		pipeline_info.pDepthStencilState = &config_info.depth_stencil_info;
		pipeline_info.pDynamicState = &config_info.dynamic_state_info;

		pipeline_info.layout = config_info.pipeline_layout;
		pipeline_info.renderPass = config_info.render_pass;
		pipeline_info.subpass = config_info.subpass;

		pipeline_info.basePipelineIndex = -1;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

		VkResult result = vkCreateGraphicsPipelines(device_.device(),
		                                            VK_NULL_HANDLE,
		                                            1,
		                                            &pipeline_info,
		                                            nullptr,
		                                            &graphics_pipeline_);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline");
		}
	}

	void vs_pipeline::create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		//this works because the char array is a vector with a standard allocator instead of c-style array.
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		if (vkCreateShaderModule(device_.device(), &createInfo, nullptr, shader_module) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module");
		}
	}

	void vs_pipeline::defaultPipelineConfigInfo(pipeline_config_info& config_info)
	{
		config_info.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		config_info.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		config_info.input_assembly_info.primitiveRestartEnable = VK_FALSE;


		config_info.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		config_info.viewport_info.viewportCount = 1;
		config_info.viewport_info.pViewports = nullptr;
		config_info.viewport_info.scissorCount = 1;
		config_info.viewport_info.pScissors = nullptr;

		config_info.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		config_info.rasterization_info.depthClampEnable = VK_FALSE;
		config_info.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
		config_info.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
		config_info.rasterization_info.lineWidth = 1.0f;
		config_info.rasterization_info.cullMode = VK_CULL_MODE_NONE;
		config_info.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		config_info.rasterization_info.depthBiasEnable = VK_FALSE;
		config_info.rasterization_info.depthBiasConstantFactor = 0.0f;
		config_info.rasterization_info.depthBiasClamp = 0.0f;
		config_info.rasterization_info.depthBiasSlopeFactor = 0.0f;

		config_info.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		config_info.multisample_info.sampleShadingEnable = VK_FALSE;
		config_info.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		config_info.multisample_info.minSampleShading = 1.0f;
		config_info.multisample_info.pSampleMask = nullptr;
		config_info.multisample_info.alphaToCoverageEnable = VK_FALSE;
		config_info.multisample_info.alphaToOneEnable = VK_FALSE;
		config_info.color_blend_attachment = {
			VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE,
			VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};

		config_info.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		config_info.color_blend_info.logicOpEnable = VK_FALSE;
		config_info.color_blend_info.logicOp = VK_LOGIC_OP_COPY;

		config_info.color_blend_info.attachmentCount = 1;
		config_info.color_blend_info.pAttachments = &config_info.color_blend_attachment;
		config_info.color_blend_info.blendConstants[0] = 0.0f;
		config_info.color_blend_info.blendConstants[1] = 0.0f;
		config_info.color_blend_info.blendConstants[2] = 0.0f;
		config_info.color_blend_info.blendConstants[3] = 0.0f;

		config_info.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		config_info.depth_stencil_info.depthTestEnable = VK_TRUE;
		config_info.depth_stencil_info.depthWriteEnable = VK_TRUE;
		config_info.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
		config_info.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
		config_info.depth_stencil_info.minDepthBounds = 0.0f;
		config_info.depth_stencil_info.maxDepthBounds = 1.0f;
		config_info.depth_stencil_info.stencilTestEnable = VK_FALSE;
		config_info.depth_stencil_info.front = {};
		config_info.depth_stencil_info.back = {};

		config_info.dynamic_state_enables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		config_info.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		config_info.dynamic_state_info.pDynamicStates = config_info.dynamic_state_enables.data();
		config_info.dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(config_info.dynamic_state_enables.
			size());
	}
}
