#include "vs_model_component.h"

#include "engine/renderer/vs_utils.h"

//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

//std
#include <cassert>
#include <iostream>
#include <unordered_map>

namespace std
{
	template <>
	struct hash<vs::vs_model_component::vertex>
	{
		std::size_t operator()(vs::vs_model_component::vertex const& vertex) const
		{
			std::size_t seed = 0;
			vs::hash_combine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace vs
{
vs_model_component::vs_model_component(vs_device& device, const vs_model_component::builder& builder) : device_(device)
	{
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

        vs_model_component::~vs_model_component()
	{
	}

	std::unique_ptr<vs_model_component>
        vs_model_component::createModelFromFile(vs_device& device, const std::string& obj_file,
	                                                        const std::string& mtl_path)
	{
		builder builder{};

		builder.loadModel(obj_file, mtl_path);
		return std::make_unique<vs_model_component>(device, builder);
	}

	void vs_model_component::createVertexBuffers(const std::vector<vertex>& vertices)
	{
		vertex_count_ = static_cast<uint32_t>(vertices.size());
		assert(vertex_count_ >= 3 && "Vertex count must be at least 3");

		VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;
		uint32_t vertex_size = sizeof(vertices[0]);

		vs_buffer staging_buffer{
			device_,
			vertex_size,
			vertex_count_,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		staging_buffer.map();
		staging_buffer.writeToBuffer((void*)vertices.data());

		vertex_buffer_ = std::make_unique<vs_buffer>(
			device_,
			vertex_size,
			vertex_count_,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		device_.copyBuffer(staging_buffer.getBuffer(), vertex_buffer_->getBuffer(), buffer_size);
	}

	void vs_model_component::createIndexBuffers(const std::vector<uint32_t>& indices)
	{
		index_count_ = static_cast<uint32_t>(indices.size());
		has_index_buffer_ = index_count_ > 0;

		if (!has_index_buffer_)return;
		VkDeviceSize buffer_size = sizeof(indices[0]) * index_count_;
		uint32_t index_size = sizeof(indices[0]);

		vs_buffer staging_buffer{
			device_, index_size, index_count_,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		staging_buffer.map();
		staging_buffer.writeToBuffer((void*)indices.data());

		index_buffer_ = std::make_unique<vs_buffer>(
			device_,
			index_size,
			index_count_,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


		device_.copyBuffer(staging_buffer.getBuffer(), index_buffer_->getBuffer(), buffer_size);
	}

	void vs_model_component::draw(VkCommandBuffer command_buffer)
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


	void vs_model_component::bind(VkCommandBuffer command_buffer)
	{
		VkBuffer buffers[] = {vertex_buffer_->getBuffer()};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);


		if (has_index_buffer_)
		{
			vkCmdBindIndexBuffer(command_buffer, index_buffer_->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	std::vector<VkVertexInputBindingDescription>
        vs_model_component::vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
		binding_descriptions[0].binding = 0;
		binding_descriptions[0].stride = sizeof(vertex);
		binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding_descriptions;
	}

	std::vector<VkVertexInputAttributeDescription>
        vs_model_component::vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

		attribute_descriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, position)});
		attribute_descriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, color)});
		attribute_descriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, normal)});
		attribute_descriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, uv)});

		return attribute_descriptions;
	}

	void vs_model_component::builder::loadModel(const std::string& obj_file, const std::string& mtr_path = "")
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
                //TODO LOAD IMAGES WITH STBI AND SAMPLE TEXTURES;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj_file.c_str(),
		                      mtr_path.empty() ? nullptr : mtr_path.c_str()))
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
					_vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2]
					};
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
