//
// Created by vstok on 2022-02-17.
//

#include "vs_texture.h"
// std
#include <cmath>
#include <stdexcept>

// lib
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "vs_buffer.h"
#include "vs_descriptors.h"

using namespace vs;

vs_texture::vs_texture(vs_device &device, const std::string &path)
    : device_(device) {
  {
    createTextureImage(path);
  };
}

std::unique_ptr<vs_texture> vs_texture::createTexture(vs_device &device,
                                                      const std::string &path) {
  return std::make_unique<vs_texture>(device, path);
}
vs_texture::~vs_texture() {
  vkDestroyImage(device_.device(), texture_image_, nullptr);
  vkFreeMemory(device_.device(), texture_memory_, nullptr);
}

void vs_texture::createTextureImage(const std::string &path) {
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels,
                              STBI_rgb_alpha);
  VkDeviceSize image_size = texWidth * texHeight * 4;
  mip_levels = static_cast<uint32_t>(
                   std::floor(std::log2(std::max(texWidth, texHeight)))) +
               1;
  if (!pixels) {
    std::runtime_error("failed to load texture image");
  }
  auto staging_buffer =
      vs_buffer(device_, image_size, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  staging_buffer.map();
  staging_buffer.writeToBuffer((void *)pixels);
VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
  VkImageCreateInfo img_create_info{};
  img_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  img_create_info.imageType = VK_IMAGE_TYPE_2D;
  img_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
  img_create_info.extent.width = static_cast<uint32_t>(texWidth);
  img_create_info.extent.height = static_cast<uint32_t>(texHeight);
  img_create_info.extent.depth = 1;
  img_create_info.mipLevels = mip_levels;
  img_create_info.arrayLayers = 1;
  img_create_info.format = format;
  img_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  img_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  img_create_info.usage =
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  img_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  img_create_info.flags = 0;

  device_.createImageWithInfo(img_create_info,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              texture_image_, texture_memory_);

  device_.transitionImageLayout(texture_image_, format,
                                img_create_info.initialLayout,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

  device_.copyBufferToImage(staging_buffer.getBuffer(), texture_image_,
                            static_cast<uint32_t>(texWidth),
                            static_cast<uint32_t>(texHeight), 1);

  generateMipmaps(texture_image_, format, texWidth, texHeight,
                  mip_levels);
}
void vs_texture::generateMipmaps(VkImage image, VkFormat image_format,
                                 int32_t texWidth, int32_t texHeight,
                                 uint32_t mipLevels) {
  VkFormatProperties format_props;
  vkGetPhysicalDeviceFormatProperties(device_.getPhysicalDevice(), image_format,
                                      &format_props);
  if (!(format_props.optimalTilingFeatures &
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    throw std::runtime_error(
        "texture image format does not support linear blitting");
  }
  VkCommandBuffer commandBuffer = device_.beginSingleTimeCommands();
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = mipLevels;
  barrier.subresourceRange.levelCount = 1;
  int32_t mipWidth = texWidth;
  int32_t mipHeight = texHeight;
  for (uint32_t i = 1; i < mipLevels; ++i) {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);

    VkImageBlit blit{};
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = i - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1,
                          mipHeight > 1 ? mipHeight / 2 : 1, 1};

    vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                   VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &barrier);

    if (mipWidth > 1)
      mipWidth /= 2;
    if (mipHeight > 1)
      mipHeight /= 2;
  }
  barrier.subresourceRange.baseMipLevel = mipLevels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);
  device_.endSingleTimeCommands(commandBuffer);
}

VkSampler vs_texture::createTextureSampler() {
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;

  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = device_.properties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;

  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.minLod = 0.0f; // static_cast<float>(mip_levels/2);
  samplerInfo.maxLod = static_cast<float>(mip_levels);
  samplerInfo.mipLodBias = 0.0f;
  VkSampler sampler;
  if (vkCreateSampler(device_.device(), &samplerInfo, nullptr, &sampler) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create texture sampler!");
  }
  return sampler;
}
VkImageView vs_texture::createImageView() {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = texture_image_;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = mip_levels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;

  if (vkCreateImageView(device_.device(), &viewInfo, nullptr, &imageView) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }

  return imageView;
}
VkDescriptorSet
vs_texture::getDescriptorset(vs_descriptor_set_layout &setLayout,
                             vs_descriptor_pool &pool) {
  VkDescriptorSet set{};
  vs_descriptor_writer(setLayout, pool)
      .writeImage(1, getImageInfo())
      .build(set);

  return set;
}
VkDescriptorImageInfo* vs_texture::getImageInfo() {
  image_info = std::make_unique<VkDescriptorImageInfo>();
  image_info->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  image_info->imageView = createImageView();
  image_info->sampler = createTextureSampler();
  return image_info.get();
}