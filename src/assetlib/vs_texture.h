//
// Created by vstok on 2022-02-17.
//

#pragma once
#include "vs_device.h"
// lib
#include "stb_image.h"
// std
#include <memory>
namespace vs {

struct TextureAsset {
  VkImage image;
  VkDeviceMemory deviceMemory;
  VkImageView view;
  VkFormat format;

  uint32_t width;
  uint32_t height;
  uint32_t numChannels;
};
struct vs_texture {
  vs_texture(vs_device &device, const std::string &path);
  ~vs_texture();

public:
  static std::unique_ptr<vs_texture>
  createTextureFromFile(vs_device &device, const std::string &path);

  VkImage createImage();
  VkImageView createImageView(VkImage image);
  static VkSampler createTextureSampler(vs_device &device, uint32_t mip_levels);
  uint32_t getMipLevels() { return mip_levels; }

private:
  void loadTextureFromFile(const std::string &path);

  void generateMipmaps(VkImage image, VkFormat image_format, int32_t texWidth,
                       int32_t texHeight, uint32_t mipLevels);

  VkDescriptorImageInfo generateImageDescriptorInfo(VkSampler sampler, VkImageView image_view);
  uint32_t texture_id;

  stbi_uc *pixel_data;
  int tex_width, tex_height, tex_channels;
  uint32_t mip_levels = 1;
  vs_device &device_;
};
} // namespace vs
  // textures
