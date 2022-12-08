#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include "vs_device.h"
#include "vs_swap_chain.h"
#include "vs_window.h"

namespace vs {
class vs_renderer {
public:
  vs_renderer(vs_window &window, vs_device &device);
  ~vs_renderer();

  vs_renderer(const vs_renderer &) = delete;
  vs_renderer &operator==(const vs_renderer &) = delete;

  [[nodiscard]] VkRenderPass getSwapChainRenderPass() const {
    return swap_chain_->getRenderPass();
  }
  float getAspectRatio() const { return swap_chain_->extentAspectRatio(); }
  bool isFrameInProgress() const { return isFrameStarted; }

  vs_swap_chain *getSwapChain() { return swap_chain_.get(); }

  VkCommandBuffer getCurrentCommandBuffer() const {
    assert(isFrameStarted &&
           "Cannot get command buffer when frame not in progress");
    return command_buffers_[currentFrameIndex];
  }

  VkCommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(VkCommandBuffer cmdBuffer);
  void endSwapChainRenderPass(VkCommandBuffer cmdBuffer);

  int getFrameIndex() const {
    assert(isFrameStarted &&
           "Cannot get frame index when frame not in progress");
    return currentFrameIndex;
  }

private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  vs_window &window_;
  vs_device &device_;
  std::unique_ptr<vs_swap_chain> swap_chain_;
  std::vector<VkCommandBuffer> command_buffers_;

  uint32_t currentImageIndex{0};
  int currentFrameIndex{0};

  bool isFrameStarted = false;
};
} // namespace vs
