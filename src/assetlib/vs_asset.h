//
// Created by Vilhelm Stokstad on 2022-02-14.
//

#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "glm/fwd.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/detail/type_mat4x4.hpp"
namespace vs {

enum class texture_format : uint32_t { Unknown = 0, RGBA8 };
enum class compression_mode : uint32_t { None, LZ4 };

struct vs_asset_file {
  char type[4];
  int version;
  std::string json;
  std::vector<char> binary_blob;
};
struct texture_info {
  uint64_t texture_size;
  texture_format format;
  compression_mode compression;
  uint32_t pixel_size[3];
  std::string original_file;
};

struct texture_asset {
  VkImage image;
  VkDeviceMemory deviceMemory;
  VkImageView view;
  VkFormat format;

  uint32_t width;
  uint32_t height;
  uint32_t numChannels;
};

compression_mode parse_compression(const char *f);

texture_info read_texture_info(vs_asset_file *asset);

void unpack_texture(texture_info *info, const char *source_buffer,
                    size_t source_size, char *destination);

vs_asset_file pack_texture(texture_info *info, void *pixel_data);



bool save_binary_file(const char *path, const vs_asset_file &file);

bool load_binary_file(const char *path, vs_asset_file &output_file);

struct vs_material {
  VkPipeline pipeline;
  VkPipelineLayout pipeline_layout;
};
struct vs_mesh{
//vs_model_component //TODO refactor the model comp to be a mesh.
};
struct render_object{
  vs_mesh* mesh;
  vs_material material;
  glm::mat4 transform_matrix;
};
} // namespace vs
