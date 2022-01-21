#include "vs_app.h"

#include "vs_model.h"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//std
#include <array>
#include <iostream>
#include <cassert>
#include <stdexcept>


float tick = 0;

namespace vs
{
	struct simple_push_constant_data
	{
		glm::vec3 offset;
		alignas(16) glm::vec3 color;
	};

	vs_app::vs_app()
	{
		loadModels();
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();
	}

	vs_app::~vs_app()
	{
		vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
	}

	void vs_app::run()
	{
		while (!window_.shouldClose())
		{
			glfwPollEvents();

			drawFrame();
			//vkDeviceWaitIdle(device_.device());

			tick += 0.016f;
		}
		vkDeviceWaitIdle(device_.device());
	}

	//TODO remove this from app when done testing.
	// just to try some stuff

	void vs_app::sierpinski(
		std::vector<vs_model::vertex>& vertices,
		int depth,
		glm::vec3 left,
		glm::vec3 right,
		glm::vec3 top,
		glm::vec3 color)
	{
		if (depth <= 0)
		{
			vertices.push_back({top, color});
			vertices.push_back({right, color});
			vertices.push_back({left, color});
		}
		else
		{
			auto leftTop = 0.5f * (left + top);
			auto rightTop = 0.5f * (right + top);
			auto leftRight = 0.5f * (left + right);
			auto newColor = color;


			sierpinski(vertices, depth - 1, left, leftRight, leftTop, newColor);
			sierpinski(vertices, depth - 1, leftRight, right, rightTop, newColor);
			sierpinski(vertices, depth - 1, leftTop, rightTop, top, newColor);
		}
	}

	void vs_app::loadModels()
	{
		std::vector<vs_model::vertex> vertices{
			{{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{0.0f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
		};

		model_ = std::make_unique<vs_model>(device_, vertices);
	}


	void vs_app::createPipelineLayout()
	{
		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(simple_push_constant_data);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &push_constant_range;

		if (vkCreatePipelineLayout(device_.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout");
		}
	}


	void vs_app::recreateSwapChain()
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
			swap_chain_ = std::make_unique<vs_swap_chain>(device_, extent, std::move(swap_chain_));
			if (swap_chain_->imageCount() != command_buffers_.size())
			{
				freeCommandBuffers();
				createCommandBuffers();
			}
		}

		// if render passes are compatible we dont need to recreate pipeline.
		createPipeline();
	}

	void vs_app::createPipeline()
	{
		assert(swap_chain_ != nullptr && "cannot create pipeline before swap chain");
		assert(pipeline_layout_ != nullptr && "cannont create pipeline before pipeline layout");

		pipeline_config_info pipeline_config{};

		vs_pipeline::defaultPipelineConfigInfo(pipeline_config);
		pipeline_config.render_pass = swap_chain_->getRenderPass();
		pipeline_config.pipeline_layout = pipeline_layout_;
		pipeline = std::make_unique<vs_pipeline>(
			device_,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipeline_config
		);
	}

	void vs_app::createCommandBuffers()
	{
		command_buffers_.resize(swap_chain_->imageCount());

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

	void vs_app::freeCommandBuffers()
	{
		vkFreeCommandBuffers(device_.device(),
		                     device_.getCommandPool(),
		                     static_cast<float>(command_buffers_.size()),
		                     command_buffers_.data());
		command_buffers_.clear();
	}

	void vs_app::recordCommandBuffer(int imageIndex)
	{
		static int frame = 0;
		frame = (frame + 1) % 1000;

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(command_buffers_[imageIndex], &begin_info) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = swap_chain_->getRenderPass();
		render_pass_begin_info.framebuffer = swap_chain_->getFrameBuffer(imageIndex);

		render_pass_begin_info.renderArea.offset = {0, 0};
		render_pass_begin_info.renderArea.extent = swap_chain_->getSwapChainExtent();

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = {{0.01f, 0.01f, 0.01f, 1.0f}};
		clear_values[1].depthStencil = {1.0f, 0};
		render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_begin_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffers_[imageIndex], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		auto extents = swap_chain_->getSwapChainExtent();
		VkViewport viewport{
			0.0f, 0.0f, static_cast<float>(extents.width), static_cast<float>(extents.height), 0.0f, 1.0f
		};
		VkRect2D scissor{{0, 0}, extents};
		vkCmdSetViewport(command_buffers_[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(command_buffers_[imageIndex], 0, 1, &scissor);

		pipeline->bind(command_buffers_[imageIndex]);

		model_->bind(command_buffers_[imageIndex]);

		for (int j = 0; j < 4; ++j)
		{
			simple_push_constant_data push{};
			push.offset = {cos(frame * 0.02) * 0.5, sin(frame * 0.02) * 0.5, 0.0f};
			push.color = {0.0f, 0.0f, 0.2f + 0.2f * j};
			vkCmdPushConstants(command_buffers_[imageIndex], pipeline_layout_,
			                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
			                   sizeof(simple_push_constant_data), &push);
			model_->draw(command_buffers_[imageIndex]);
		}


		vkCmdEndRenderPass(command_buffers_[imageIndex]);

		if (vkEndCommandBuffer(command_buffers_[imageIndex]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to end recording cmd buffer");
		}
	}

	void vs_app::drawFrame()
	{
		uint32_t image_idx;
		auto result = swap_chain_->acquireNextImage(&image_idx);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain index");
		}
		recordCommandBuffer(image_idx);
		result = swap_chain_->submitCommandBuffers(&command_buffers_[image_idx], &image_idx);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR == window_.wasFrameBufferResized())
		{
			window_.resetFrameBufferResizedFlag();
			recreateSwapChain();
			return;
		}
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit command buffers at index: " + std::to_string(image_idx));
		}
	}
}
