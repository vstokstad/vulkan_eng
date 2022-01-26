#include "vs_renderer.h"


//std
#include <array>
#include <iostream>
#include <cassert>
#include <stdexcept>


namespace vs
{
	vs_renderer::vs_renderer(vs_window& window, vs_device& device) : window_(window), device_(device)
	{
		recreateSwapChain();
		createCommandBuffers();
	}

	vs_renderer::~vs_renderer()
	{
		freeCommandBuffers();
	}

	void vs_renderer::recreateSwapChain()
	{
		auto extent = window_.getExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = window_.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(device_.device());

		if (swap_chain_ == nullptr)
		{
			swap_chain_ = std::make_unique<vs_swap_chain>(device_, extent);
		}
		else
		{
			std::shared_ptr<vs_swap_chain> old_swap_chain = std::move(swap_chain_);
			swap_chain_ = std::make_unique<vs_swap_chain>(device_, extent, old_swap_chain);

			if (!old_swap_chain->compareSwapFormats(*swap_chain_.get()))
			{
				throw std::runtime_error("swapchains have non matching formats");
			}
		}
	}


	void vs_renderer::createCommandBuffers()
	{
		command_buffers_.resize(vs_swap_chain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = device_.getCommandPool();
		alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

		if (vkAllocateCommandBuffers(device_.device(), &alloc_info, command_buffers_.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers");
		}
	}

	void vs_renderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(device_.device(),
		                     device_.getCommandPool(),
		                     static_cast<uint32_t>(command_buffers_.size()),
		                     command_buffers_.data());
		command_buffers_.clear();
	}


	VkCommandBuffer vs_renderer::beginFrame()
	{
		assert(!isFrameStarted && "can't call beginFrame while already in progress");

		auto result = swap_chain_->acquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return nullptr;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain index");
		}

		isFrameStarted = true;

		auto command_buffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		return command_buffer;
	}

	void vs_renderer::endFrame()
	{
		assert(isFrameStarted && "Can't call endFrame while not in progrsess");
		auto command_buffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to end recording cmd buffer");
		}

		auto result = swap_chain_->submitCommandBuffers(&command_buffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR == window_.wasFrameBufferResized())
		{
			window_.resetFrameBufferResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit command buffers at index: " + std::to_string(currentImageIndex));
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % vs_swap_chain::MAX_FRAMES_IN_FLIGHT;
	}

	void vs_renderer::beginSwapChainRenderPass(VkCommandBuffer cmdBuffer)
	{
		assert(isFrameStarted && "Cannot begin render pass when frame is in progress");
		assert(
			cmdBuffer == getCurrentCommandBuffer() &&
			"can't begin render pass on command buffer from a different fram");

		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = swap_chain_->getRenderPass();
		render_pass_begin_info.framebuffer = swap_chain_->getFrameBuffer(currentImageIndex);

		render_pass_begin_info.renderArea.offset = {0, 0};
		render_pass_begin_info.renderArea.extent = swap_chain_->getSwapChainExtent();

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = {{0.01f, 0.01f, 0.01f, 1.0f}};
		clear_values[1].depthStencil = {1.0f, 0};
		render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_begin_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(cmdBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		auto extents = swap_chain_->getSwapChainExtent();
		VkViewport viewport{
			0.0f, 0.0f, static_cast<float>(swap_chain_->getSwapChainExtent().width),
			static_cast<float>(swap_chain_->getSwapChainExtent().height), 0.0f, 1.0f
		};
		VkRect2D scissor{{0, 0}, swap_chain_->getSwapChainExtent()};
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
	}

	void vs_renderer::endSwapChainRenderPass(VkCommandBuffer cmdBuffer)
	{
		assert(isFrameStarted && "Cannot end render pass when frame is not in progress");
		assert(
			cmdBuffer == getCurrentCommandBuffer() &&
			"can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(cmdBuffer);
	}
}
