// vulkan_eng -> vulkan_test -> vs_pipeline.h 
// 2022 © Vilhelm Stokstad

#pragma once
#include <string>
#include <vector>
#include "vs_device.h"

namespace vs
{
	struct pipeline_config_info
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

	class vs_pipeline
	{
	public:
		vs_pipeline(vs_device& device,
		            const std::string& vert_path,
		            const std::string& frag_path,
		            const pipeline_config_info& config);

		~vs_pipeline();

		vs_pipeline(const vs_pipeline&) = delete;

		void operator=(const vs_pipeline&) = delete;

		void bind(VkCommandBuffer command_buffer);

		static pipeline_config_info defaultPipelineConfigInfo(uint32_t width, uint32_t height);

	private:
		static std::vector<char> readFile(const std::string& path);

		void create_graphics_pipeline(const std::string& vert_path,
		                              const std::string& frag_path,
		                              const pipeline_config_info& config_info);

		void create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module);

		vs_device& device_;
		VkPipeline graphics_pipeline_;
		VkShaderModule vert_shader_module_;
		VkShaderModule frag_shader_module_;
	};
}
