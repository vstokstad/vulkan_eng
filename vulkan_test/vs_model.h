#pragma once
#include "vs_device.h"

#define GLM_FORCE_RADIANS
#define GLF_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>

namespace vs
{
	class vs_model
	{
	public:
		struct vertex
		{
			glm::vec3 position;
			glm::vec3 color;
			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		vs_model(vs_device& device, const std::vector<vertex>& vertices);
		~vs_model();
		vs_model(const vs_model&) = delete;
		vs_model& operator=(const vs_model&) = delete;

		void bind(VkCommandBuffer command_buffer);
		void draw(VkCommandBuffer command_buffer);


	private:
		void createVertexBuffers(const std::vector<vertex>& vertices);

		vs_device& device_;
		VkBuffer vertex_buffer_;
		VkDeviceMemory vertex_buffer_memory_;
		uint32_t vertex_count;
	};
}
