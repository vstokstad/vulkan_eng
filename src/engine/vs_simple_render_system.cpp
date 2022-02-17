#include "vs_simple_render_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cassert>
#include <stdexcept>

namespace vs {
struct simple_push_constant_data {
  glm::mat4 model_matrix{1.f};
  glm::mat4 normal_matrix{1.0f};
};

vs_simple_render_system::vs_simple_render_system(
    vs_device &device, VkRenderPass render_pass,
    VkDescriptorSetLayout global_set_layout)
    : device_(device) {
  createPipelineLayout(global_set_layout);
  createPipeline(render_pass);
}

vs_simple_render_system::~vs_simple_render_system() {
  vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
}

void vs_simple_render_system::createPipelineLayout(
    VkDescriptorSetLayout global_set_layout) {
  VkPushConstantRange push_constant_range{};
  push_constant_range.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push_constant_range.offset = 0;
  push_constant_range.size = sizeof(simple_push_constant_data);

  std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount =
      static_cast<uint32_t>(descriptor_set_layouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptor_set_layouts.data();

  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &push_constant_range;

  if (vkCreatePipelineLayout(device_.device(), &pipelineLayoutInfo, nullptr,
                             &pipeline_layout_) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout");
  }
}

void vs_simple_render_system::createPipeline(VkRenderPass render_pass) {
  assert(pipeline_layout_ != nullptr &&
         "cannont create pipeline before pipeline layout");

  pipeline_config_info pipeline_config{};

  vs_pipeline::defaultPipelineConfigInfo(pipeline_config, device_.msaa_samples,
                                         true);

  pipeline_config.render_pass = render_pass;
  pipeline_config.pipeline_layout = pipeline_layout_;
  pipeline = std::make_unique<vs_pipeline>(
      device_, "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv", pipeline_config);
}

void vs_simple_render_system::renderGameObjects(frame_info &frame_info) {
  pipeline->bind(frame_info.command_buffer);

  vkCmdBindDescriptorSets(frame_info.command_buffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0,
                          1, &frame_info.global_descriptor_set, 0, nullptr);

  for (auto &kv : frame_info.game_objects) {
    auto &object = kv.second;
    if (object.model_comp == nullptr)
      continue;

    if (object.model_texture!=nullptr){
      VkDescriptorImageInfo img_info{};
      img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      img_info.imageView =
          object.model_texture->createImageView();
      img_info.sampler =object.model_texture->createTextureSampler();

      vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1, &frame_info.global_descriptor_set, 0, nullptr);
    }
    simple_push_constant_data push{};
    push.model_matrix = object.transform_comp.mat4();
    push.normal_matrix = object.transform_comp.normal_matrix();
    vkCmdPushConstants(frame_info.command_buffer, pipeline_layout_,
                       VK_SHADER_STAGE_VERTEX_BIT |
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(simple_push_constant_data), &push);

    object.model_comp->bind(frame_info.command_buffer);
    object.model_comp->draw(frame_info.command_buffer);
  }
}
} // namespace vs
