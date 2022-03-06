// vulkan_eng -> vulkan_test -> vs_pipeline.h
// 2022 © Vilhelm Stokstad

#pragma once
#include "vs_device.h"
// std
#include <string>
#include <vector>

namespace vs {
struct pipeline_config_info {
  pipeline_config_info() = default;
  pipeline_config_info(const pipeline_config_info &) = delete;
  pipeline_config_info &operator=(const pipeline_config_info &) = delete;

  std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions{};
  std::vector<VkVertexInputAttributeDescription>
      vertex_attribute_descriptions{};
  std::vector<VkDescriptorImageInfo> image_descriptor_info;
  VkPipelineViewportStateCreateInfo viewport_info;
  VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
  VkPipelineRasterizationStateCreateInfo rasterization_info;
  VkPipelineMultisampleStateCreateInfo multisample_info;
  VkPipelineColorBlendAttachmentState color_blend_attachment;
  VkPipelineColorBlendStateCreateInfo color_blend_info;
  VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
  std::vector<VkDynamicState> dynamic_state_enables;
  VkPipelineDynamicStateCreateInfo dynamic_state_info;
  VkPipelineLayout pipeline_layout = nullptr;
  VkRenderPass render_pass = nullptr;
  uint32_t subpass = 0;
};

class vs_pipeline {
public:
  vs_pipeline(vs_device &device, const std::string &vert_path,
              const std::string &frag_path, const pipeline_config_info &config);

  ~vs_pipeline();

  vs_pipeline(const vs_pipeline &) = delete;
  vs_pipeline &operator=(const vs_pipeline &) = delete;

  void bind(VkCommandBuffer command_buffer);

  static void defaultPipelineConfigInfo(pipeline_config_info &config_info,
                                        VkSampleCountFlagBits msaa_samples,
                                        bool enable_sample_shading);

private:
  static std::vector<char> readFile(const std::string &path);

  void create_graphics_pipeline(const std::string &vert_path,
                                const std::string &frag_path,
                                const pipeline_config_info &config_info);

  void create_shader_module(const std::vector<char> &code,
                            VkShaderModule *shader_module);

  vs_device &device_;
  VkPipeline graphics_pipeline_;
  VkShaderModule vert_shader_module_;
  VkShaderModule frag_shader_module_;
};
} // namespace vs
