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
  glm::mat4 inv_view_mat{0};
  glm::vec4 ambient_light_color{1.f, 1.f, 1.f, 0.1f}; // w is intensity
  int num_lights;
  point_light point_lights[MAX_LIGHTS];
};
// below is just a better struct when we are using more ubo than the global,
// sort of.
struct vs_scene_data {
  glm::vec4 fog_color;     // w is exponent;
  glm::vec4 fog_distances; // x for min, y for max, zw unused
  glm::vec4 ambient_color;
  glm::vec4 sunlight_direction; // w for power/strength
  glm::vec4 sunlight_color;
};
struct frame_info {
  int frame_index;
  float frame_time;

  static constexpr float fixed_time_step = 1.0f / 60.f;

  VkCommandBuffer command_buffer;
  vs_camera &camera;
  VkDescriptorSet global_descriptor_set;
  std::vector<VkDescriptorSet> descriptor_sets; // TODO this could be used for all of them.
  vs_game_object::map &game_objects;
  vs_game_object::map &lights;
};
} // namespace vs
