//
// Created by vstok on 2022-02-17.
//

#pragma once

#include "vs_renderer.h"
#include <vulkan/vulkan_core.h>
namespace vs {

struct vs_texture {
  std::string path = "models/textures/viking_room.png";


  vs_texture(vs_device &device, const std::string &path);
  ~vs_texture();

  static std::shared_ptr<vs_texture> createTexture(vs_device& device, const std::string &path);

  VkDescriptorImageInfo getTextureImageInfo() {
    image_info.sampler = texture_sampler_;
    image_info.imageView = texture_view_;
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    return image_info;
  };
  VkImageView getImageView() const;
  VkSampler getSampler() const;
  VkImage getImage() const;

private:
  VkDescriptorImageInfo image_info{};
  VkImage texture_image_;
  VkDeviceMemory texture_memory_;
  VkImageView texture_view_;
  VkSampler texture_sampler_;

  void createTextureImage(const std::string &path);
  void generateMipmaps(VkImage image, VkFormat image_format, int32_t texWidth,
                       int32_t texHeight, uint32_t mipLevels);

  VkSampler createTextureSampler();
  VkImageView createImageView(VkImage image, VkImageAspectFlags aspect_mask,
                              VkFormat format);
  uint32_t mip_levels = 1;
  vs_device &device_;
};
} // namespace vs
  // textures
