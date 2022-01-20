#include "vs_app.h"

#include <array>
#include <stdexcept>

#include "vs_model.h"

namespace vs
{
	vs_app::vs_app()
	{
		loadModels();
		createPipelineLayout();
		createPipeline();
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
		}
		vkDeviceWaitIdle(device_.device());
	}

	void vs_app::sierpinski(
		std::vector<vs_model::vertex>& vertices,
		int depth,
		glm::vec2 left,
		glm::vec2 right,
		glm::vec2 top,
		glm::vec3 color)
	{
		glm::vec3 r{1.0f, 0.0f, 0.0f};
		glm::vec3 g{0.0f, 1.0f, 0.0f};
		glm::vec3 b{0.0f, 0.0f, 1.0f};

		if (depth <= 0)
		{
			vertices.push_back({top, r});
			vertices.push_back({right, g});
			vertices.push_back({left, b});
		}
		else
		{
			auto leftTop = 0.5f * (left + top);
			auto rightTop = 0.5f * (right + top);
			auto leftRight = 0.5f * (left + right);
			auto newR = 0.5f * (r + b);
			auto newg = 0.5f * (g + 0.5f * (r + b));
			auto newb = 0.5f * (b + newR);
			sierpinski(vertices, depth - 1, left, leftRight, leftTop, newR);
			sierpinski(vertices, depth - 1, leftRight, right, rightTop, newg);
			sierpinski(vertices, depth - 1, leftTop, rightTop, top, newb);
		}
	}

	void vs_app::loadModels()
	{
		std::vector<vs_model::vertex> vertices{};

		sierpinski(vertices, 2, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f}, {1.0f, 0.0f, 0.0f});
		model_ = std::make_unique<vs_model>(device_, vertices);
	}

	void vs_app::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(device_.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void vs_app::createPipeline()
	{
		auto pipline_config = vs_pipeline::defaultPipelineConfigInfo(swap_chain_.width(), swap_chain_.height());
		pipline_config.render_pass = swap_chain_.getRenderPass();
		pipline_config.pipeline_layout = pipeline_layout_;
		pipeline = std::make_unique<vs_pipeline>(
			device_,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipline_config
		);
	}

	void vs_app::createCommandBuffers()
	{
		command_buffers_.resize(swap_chain_.imageCount());

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = device_.getCommandPool();
		alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

		if (vkAllocateCommandBuffers(device_.device(), &alloc_info, command_buffers_.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers");
		}

		for (int i = 0; i < command_buffers_.size(); ++i)
		{
			VkCommandBufferBeginInfo begin_info{};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			if (vkBeginCommandBuffer(command_buffers_[i], &begin_info) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to begin recording command buffer!");
			}
			VkRenderPassBeginInfo render_pass_begin_info{};
			render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.renderPass = swap_chain_.getRenderPass();
			render_pass_begin_info.framebuffer = swap_chain_.getFrameBuffer(i);

			render_pass_begin_info.renderArea.offset = {0, 0};
			render_pass_begin_info.renderArea.extent = swap_chain_.getSwapChainExtent();

			std::array<VkClearValue, 2> clear_values{};
			clear_values[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
			//clear_values[0].depthStencil = {1.0f, 0};
			//clear_values[1].color = {{1.0f, 0.0f, 0.0f, 1.0f}};
			clear_values[1].depthStencil = {1.0f, 0};
			render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
			render_pass_begin_info.pClearValues = clear_values.data();

			vkCmdBeginRenderPass(command_buffers_[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			pipeline->bind(command_buffers_[i]);

			model_->bind(command_buffers_[i]);
			model_->draw(command_buffers_[i]);

			vkCmdEndRenderPass(command_buffers_[i]);

			if (vkEndCommandBuffer(command_buffers_[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to end recording cmd buffer");
			}
		}
	}

	void vs_app::drawFrame()
	{
		uint32_t image_idx;
		auto result = swap_chain_.acquireNextImage(&image_idx);
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain index");
		}
		result = swap_chain_.submitCommandBuffers(&command_buffers_[image_idx], &image_idx);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit command buffers at index: " + std::to_string(image_idx));
		}
	}
}
