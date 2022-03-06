#pragma once
#include "vs_camera.h"
#include "vs_game_object.h"
#include <vulkan/vulkan.h>

namespace vs {
#define MAX_LIGHTS 10

struct point_light {
  glm::vec4 position; // ignore w
  glm::vec4 color;    // w is intensity
};

struct global_ubo {
  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};
  glm::mat4 inv_view_mat{1.f};
  glm::vec3 ambient_light_color{0.1f};
  int num_lights;
  point_light point_lights[MAX_LIGHTS];
};

struct frame_info {
  int frame_index;
  float frame_time;

  VkCommandBuffer command_buffer;
  VkDescriptorSet global_descriptor_set;
  vs_game_object::map &game_objects;
  vs_game_object::map &lights;
};
} // namespace vs
