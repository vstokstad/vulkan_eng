#pragma once

#include <memory>

#include "engine/renderer/vs_device.h"
#include "engine/renderer/vs_pipeline.h"
#include "engine/vs_frame_info.h"

namespace vs {
class vs_simple_physics_system{
public:
  vs_simple_physics_system(vs_device &device, VkRenderPass render_pass,
                               VkDescriptorSetLayout global_set_layout);
  ~vs_simple_physics_system();

  vs_simple_physics_system(const vs_simple_physics_system &) = delete;
  vs_simple_physics_system &
  operator==(const vs_simple_physics_system &) = delete;

  void update(const frame_info &frame_info, global_ubo &ubo);
  void render(frame_info &frame_info);

  glm::vec3 gravity{0.0f,9.81f, 0.0f};


private:
  void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
  void createPipeline(VkRenderPass render_pass);

  vs_device &device_;
  std::unique_ptr<vs_pipeline> pipeline;
  VkPipelineLayout pipeline_layout_;
};
}
