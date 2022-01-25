#include "vs_model.h"

#include "vs_utils.h"

//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

//std
#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>

namespace std
{
	template <>
	struct hash<vs::vs_model::vertex>
	{
		std::size_t operator()(vs::vs_model::vertex const& vertex) const
		{
			std::size_t seed = 0;
			vs::hash_combine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace vs
{
	vs_model::vs_model(vs_device& device, const vs_model::builder& builder) : device_(device)
	{
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	vs_model::~vs_model()
	{
		vkDestroyBuffer(device_.device(), vertex_buffer_, nullptr);
		vkFreeMemory(device_.device(), vertex_buffer_memory_, nullptr);

		if (has_index_buffer_)
		{
			vkDestroyBuffer(device_.device(), index_buffer_, nullptr);
			vkFreeMemory(device_.device(), index_buffer_memory_, nullptr);
		}
	}

	std::unique_ptr<vs_model> vs_model::createModelFromFile(vs_device& device, const std::string& filepath)
	{
		builder builder{};
		builder.loadModel(filepath);
		return std::make_unique<vs_model>(device, builder);
	}

	void vs_model::createVertexBuffers(const std::vector<vertex>& vertices)
	{
		vertex_count_ = static_cast<uint32_t>(vertices.size());
		assert(vertex_count_ >= 3 && "Vertex count must be at least 3");

		VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;
		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;

		device_.createBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging_buffer,
			staging_buffer_memory);

		void* data;
		vkMapMemory(device_.device(), staging_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
		vkUnmapMemory(device_.device(), staging_buffer_memory);

		device_.createBuffer(
			buffer_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer_,
			vertex_buffer_memory_);

		device_.copyBuffer(staging_buffer, vertex_buffer_, buffer_size);

		vkDestroyBuffer(device_.device(), staging_buffer, nullptr);
		vkFreeMemory(device_.device(), staging_buffer_memory, nullptr);
	}

	void vs_model::createIndexBuffers(const std::vector<uint32_t>& indices)
	{
		index_count_ = static_cast<uint32_t>(indices.size());
		has_index_buffer_ = index_count_ > 0;

		if (!has_index_buffer_)return;


		VkDeviceSize buffer_size = sizeof(indices[0]) * index_count_;
		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;

		device_.createBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging_buffer,
			staging_buffer_memory);

		void* data;
		vkMapMemory(device_.device(), staging_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(buffer_size));
		vkUnmapMemory(device_.device(), staging_buffer_memory);

		device_.createBuffer(
			buffer_size,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer_,
			index_buffer_memory_);

		device_.copyBuffer(staging_buffer, index_buffer_, buffer_size);

		vkDestroyBuffer(device_.device(), staging_buffer, nullptr);
		vkFreeMemory(device_.device(), staging_buffer_memory, nullptr);
	}

	void vs_model::draw(VkCommandBuffer command_buffer)
	{
		if (has_index_buffer_)
		{
			vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);
		}
		else
		{
			vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
		}
	}


	void vs_model::bind(VkCommandBuffer command_buffer)
	{
		VkBuffer buffers[] = {vertex_buffer_};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);


		if (has_index_buffer_)
		{
			vkCmdBindIndexBuffer(command_buffer, index_buffer_, 0, VK_INDEX_TYPE_UINT32);
		}
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
			{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, position)},
			{1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, color)},

		};
	}

	void vs_model::builder::loadModel(const std::string& filename)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
		{
			throw std::runtime_error(warn + err);
		}

		vertices.clear();
		indices.clear();

		std::unordered_map<vertex, uint32_t> unique_vertices{};

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				vertex _vertex{};
				if (index.vertex_index >= 0)
				{
					_vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					auto color_index = 3 * index.vertex_index + 2;
					if (color_index < static_cast<int>(attrib.colors.size()))
					{
						_vertex.color = {
							attrib.colors[color_index - 2],
							attrib.colors[color_index - 1],
							attrib.colors[color_index - 0],
						};
					}
					else
					{
						_vertex.color = {1.f, 1.f, 1.f};
					}
				}

				if (index.normal_index >= 0)
				{
					_vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}

				if (index.texcoord_index >= 0)
				{
					_vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1],
					};
				}

				if (!unique_vertices.contains(_vertex))
				{
					unique_vertices[_vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(_vertex);
				}
				indices.push_back(unique_vertices[_vertex]);
			}
		}
	}
}
