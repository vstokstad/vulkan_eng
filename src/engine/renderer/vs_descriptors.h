#pragma once

#include "vs_device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace vs
{
	class vs_descriptor_set_layout
	{
	public:
		class vs_builder
		{
		public:
			explicit vs_builder(vs_device& device) : device_{device}
			{
			}

			vs_builder& addBinding(
				uint32_t binding,
				VkDescriptorType descriptorType,
				VkShaderStageFlags stageFlags,
				uint32_t count = 1);

			std::unique_ptr<vs_descriptor_set_layout> build() const;

		private:
			vs_device& device_;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_{};
		};

		vs_descriptor_set_layout(
			vs_device& lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
		~vs_descriptor_set_layout();
		vs_descriptor_set_layout(const vs_descriptor_set_layout&) = delete;
		vs_descriptor_set_layout& operator=(const vs_descriptor_set_layout&) = delete;

		VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptor_set_layout_; }

	private:
		vs_device& device_;
		VkDescriptorSetLayout descriptor_set_layout_;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_;

		friend class vs_descriptor_writer;
	};

	class vs_descriptor_pool
	{
	public:
		class vs_builder
		{
		public:
			vs_builder(vs_device& device) : device_{device}
			{
			}

			vs_builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
			vs_builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
			vs_builder& setMaxSets(uint32_t count);
			std::unique_ptr<vs_descriptor_pool> build() const;

		private:
			vs_device& device_;
			std::vector<VkDescriptorPoolSize> pool_sizes_{};
			uint32_t max_sets_ = 1000;
			VkDescriptorPoolCreateFlags pool_flags_ = 0;
		};

		vs_descriptor_pool(
			vs_device& device,
			uint32_t maxSets,
			VkDescriptorPoolCreateFlags poolFlags,
			const std::vector<VkDescriptorPoolSize>& poolSizes);
		~vs_descriptor_pool();
		vs_descriptor_pool(const vs_descriptor_pool&) = delete;
		vs_descriptor_pool& operator=(const vs_descriptor_pool&) = delete;

		bool allocateDescriptor(
			const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

		void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

		void resetPool();

	private:
		vs_device& device_;
		VkDescriptorPool descriptor_pool_;

		friend class vs_descriptor_writer;
	};

	class vs_descriptor_writer
	{
	public:
		vs_descriptor_writer(vs_descriptor_set_layout& setLayout, vs_descriptor_pool& pool);

		vs_descriptor_writer& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		vs_descriptor_writer& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

		bool build(VkDescriptorSet& set);
		void overwrite(VkDescriptorSet& set);

	private:
		vs_descriptor_set_layout& set_layout_;
		vs_descriptor_pool& pool_;
		std::vector<VkWriteDescriptorSet> writes_;
	};
} // namespace lve
