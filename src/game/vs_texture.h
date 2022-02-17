//
// Created by vstok on 2022-02-17.
//

#pragma once

#include "vs_buffer.h"
#include "vs_renderer.h"
#include "vs_descriptors.h"
#include <vulkan/vulkan_core.h>
namespace vs {

struct vs_texture {
  vs_texture(vs_device &device, const std::string &path);
  ~vs_texture();

public:
  static std::unique_ptr<vs_texture> createTexture(vs_device &device,
                                                   const std::string &path);

  VkSampler createTextureSampler();

  VkImageView createImageView();
  VkDescriptorSet getDescriptorset(vs_descriptor_set_layout &setLayout,
                                   vs_descriptor_pool &pool);
  VkDescriptorImageInfo* getImageInfo();

private:
  VkImage texture_image_;
  VkDeviceMemory texture_memory_;
std::unique_ptr<VkDescriptorImageInfo> image_info;

  void createTextureImage(const std::string &path);

  void generateMipmaps(VkImage image, VkFormat image_format, int32_t texWidth,
                       int32_t texHeight, uint32_t mipLevels);



  uint32_t mip_levels = 1;
  vs_device &device_;
};
} // namespace vs
  // textures
