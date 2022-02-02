#pragma once
#include "vs_buffer.h"
#include "vs_device.h"
// libs
#define GLM_FORCE_RADIANS
#define GLF_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace vs {
class vs_model_component {
public:
  struct vertex {
    glm::vec3 position{};
    glm::vec3 color{};
    glm::vec3 normal{};
    glm::vec2 uv{};

    static std::vector<VkVertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();

    bool operator==(const vertex &other) const {
      return position == other.position && color == other.color &&
             normal == other.normal && uv == other.uv;
    }
  };

  struct builder {
    std::vector<vertex> vertices{};
    std::vector<uint32_t> indices{};
    std::string name;

    void loadModel(const std::string &obj_file, const std::string &mtr_path, bool normalize_scale = true);
  };

  vs_model_component(vs_device &device, const builder &builder);
  ~vs_model_component();

  vs_model_component(const vs_model_component &) = delete;
  vs_model_component &operator=(const vs_model_component &) = delete;

  static std::unique_ptr<vs_model_component>
  createModelFromFile(vs_device &device, const std::string &obj_file,
                      const std::string &mtl_path = nullptr);

  void bind(VkCommandBuffer command_buffer);
  void draw(VkCommandBuffer command_buffer);

  std::string string_name;

private:
  void createVertexBuffers(const std::vector<vertex> &vertices);
  void createIndexBuffers(const std::vector<uint32_t> &indices);

  vs_device &device_;

  std::unique_ptr<vs_buffer> vertex_buffer_;
  uint32_t vertex_count_ = 0;

  bool has_index_buffer_ = false;
  std::unique_ptr<vs_buffer> index_buffer_;
  uint32_t index_count_ = 0;

};
}; // namespace vs