#include "vs_model.h"

#include <cassert>
#include <cstring>

namespace vs
{
	vs_model::vs_model(vs_device& device, const std::vector<vertex>& vertices) : device_(device)
	{
		createVertexBuffers(vertices);
	}

	vs_model::~vs_model()
	{
		vkDestroyBuffer(device_.device(), vertex_buffer_, nullptr);
		vkFreeMemory(device_.device(), vertex_buffer_memory_, nullptr);
	}

	void vs_model::createVertexBuffers(const std::vector<vertex>& vertices)
	{
		vertex_count = static_cast<uint32_t>(vertices.size());
		assert(vertex_count >= 3 && "Vertex count must be at least 3");
		VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count;
		device_.createBuffer(
			buffer_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertex_buffer_,
			vertex_buffer_memory_);

		void* data;
		vkMapMemory(device_.device(), vertex_buffer_memory_, 0, buffer_size, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
		vkUnmapMemory(device_.device(), vertex_buffer_memory_);
	}

	void vs_model::draw(VkCommandBuffer command_buffer)
	{
		vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
	}

	void vs_model::bind(VkCommandBuffer command_buffer)
	{
		VkBuffer buffers[] = {vertex_buffer_};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
	}

	std::vector<VkVertexInputBindingDescription> vs_model::vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
		binding_descriptions[0].binding = 0;
		binding_descriptions[0].stride = sizeof(vertex);
		binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding_descriptions;
	}

	std::vector<VkVertexInputAttributeDescription> vs_model::vertex::getAttributeDescriptions()
	{
		return {
			{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, position)},
			{1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, color)},

		};
	}
}
