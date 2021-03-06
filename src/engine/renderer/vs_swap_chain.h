#pragma once

#include "vs_device.h"

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace vs {
class vs_swap_chain {
public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 1;

  vs_swap_chain(vs_device &deviceRef, VkExtent2D windowExtent);
  vs_swap_chain(vs_device &deviceRef, VkExtent2D windowExtent,
                std::shared_ptr<vs_swap_chain> previous_swap_chain);
  ~vs_swap_chain();

  vs_swap_chain(const vs_swap_chain &) = delete;
  vs_swap_chain operator=(const vs_swap_chain &) = delete;

  VkFramebuffer getFrameBuffer(int index) {
    return swapChainFramebuffers[index];
  }
  VkRenderPass getRenderPass() { return renderPass; }
  VkImageView getImageView(int index) { return swapChainImageViews[index]; }
  size_t imageCount() { return swapChainImages.size(); }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
  VkExtent2D getSwapChainExtent() { return swapChainExtent; }
  uint32_t width() { return swapChainExtent.width; }
  uint32_t height() { return swapChainExtent.height; }

  float extentAspectRatio() {
    return static_cast<float>(swapChainExtent.width) /
           static_cast<float>(swapChainExtent.height);
  }

  VkFormat findDepthFormat();

  VkResult acquireNextImage(uint32_t *imageIndex);
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers,
                                uint32_t *imageIndex);

  bool compareSwapFormats(const vs_swap_chain &swap_chain) const {
    return swap_chain.swapChainDepthFormat == swapChainDepthFormat &&
           swap_chain.swapChainImageFormat == swapChainImageFormat;
  }

private:
  void init();
  void createSwapChain();
  void createImageViews();

  void createColorResources(); //msaa
  void createDepthResources();
  void createRenderPass();
  void createFramebuffers();
  void createSyncObjects();

  VkImageView createImageView(VkImage image, VkImageAspectFlags aspect_mask,
                              VkFormat format, uint32_t mipLevels);

  //** TEXTURES **//
  void createTextureImage(const std::string &path);
  void createImage(uint32_t width, uint32_t height, uint32_t mip_levels,
                   VkSampleCountFlagBits num_samples, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, VkImage &image,
                   VkDeviceMemory &imageMemory);

  void createTextureImageView();
  void createTextureSampler();



  // Helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat swapChainImageFormat;
  VkFormat swapChainDepthFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkRenderPass renderPass;

  std::vector<VkImage> depthImages;
  std::vector<VkDeviceMemory> depthImageMemorys;
  std::vector<VkImageView> depthImageViews;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  VkImage colorImage;
  VkDeviceMemory colorImageMemory;
  VkImageView colorImageView;

public:
  // TEXTURES//

  VkImageView getSwapChainTextureImageView() { return textureImageView; }
  VkSampler getSwapChainTextureSampler() { return textureSampler; }
  uint32_t mip_levels;
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;
  std::vector<VkImage> swapChainTextureImage;
  std::vector<VkImageView> swapChainTextureImageViews;

private:
  vs_device &device;
  VkExtent2D windowExtent;

  VkSwapchainKHR swapChain;
  std::shared_ptr<vs_swap_chain> old_swap_chain;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;
  void generateMipmaps(VkImage image, VkFormat image_format, int32_t texWidth,
                       int32_t texHeight, uint32_t mipLevels);
};
} // namespace vs
