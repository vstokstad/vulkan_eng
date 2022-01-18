#pragma once

struct pipeline_config_info_t
{
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
	VkPipelineRasterizationStateCreateInfo rasterization_info;
	VkPipelineMultisampleStateCreateInfo multisample_info;
	VkPipelineColorBlendAttachmentState color_blend_attachment;
	VkPipelineColorBlendStateCreateInfo color_blend_info;
	VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
	VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
	VkRenderPass render_pass = VK_NULL_HANDLE;
	uint32_t subpass = 0;
};
