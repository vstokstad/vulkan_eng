#pragma once

#include "vs_device.h"


// std lib headers
#include <string>
#include <memory>
#include <vector>

namespace vs
{
	class vs_swap_chain
	{
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 1;

		vs_swap_chain(vs_device& deviceRef, VkExtent2D windowExtent);
		vs_swap_chain(vs_device& deviceRef, VkExtent2D windowExtent,
		              std::shared_ptr<vs_swap_chain> previous_swap_chain);
		~vs_swap_chain();

		vs_swap_chain(const vs_swap_chain&) = delete;
		vs_swap_chain operator=(const vs_swap_chain&) = delete;

		VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
		VkRenderPass getRenderPass() { return renderPass; }
		VkImageView getImageView(int index) { return swapChainImageViews[index]; }
		size_t imageCount() { return swapChainImages.size(); }
		VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
		VkExtent2D getSwapChainExtent() { return swapChainExtent; }
		uint32_t width() { return swapChainExtent.width; }
		uint32_t height() { return swapChainExtent.height; }

		float extentAspectRatio()
		{
			return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
		}

		VkFormat findDepthFormat();

		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

		bool compareSwapFormats(const vs_swap_chain& swap_chain) const
		{
			return swap_chain.swapChainDepthFormat == swapChainDepthFormat &&
				swap_chain.swapChainImageFormat == swapChainImageFormat;
		}

	private:
		void init();
		void createSwapChain();
		void createImageViews();
		void createDepthResources();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		// Helper functions
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(
			const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(
			const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

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

		vs_device& device;
		VkExtent2D windowExtent;

		VkSwapchainKHR swapChain;
		std::shared_ptr<vs_swap_chain> old_swap_chain;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;
	};
} // namespace lve
