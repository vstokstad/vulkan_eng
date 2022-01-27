#include "vs_descriptors.h"


// std
#include <cassert>
#include <stdexcept>

namespace vs
{
	// *************** Descriptor Set Layout vs_builder *********************

	vs_descriptor_set_layout::vs_builder& vs_descriptor_set_layout::vs_builder::addBinding(
		uint32_t binding,
		VkDescriptorType descriptorType,
		VkShaderStageFlags stageFlags,
		uint32_t count)
	{
		assert(bindings_.count(binding) == 0 && "Binding already in use");
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;
		bindings_[binding] = layoutBinding;
		return *this;
	}

	std::unique_ptr<vs_descriptor_set_layout> vs_descriptor_set_layout::vs_builder::build() const
	{
		return std::make_unique<vs_descriptor_set_layout>(device_, bindings_);
	}

	// *************** Descriptor Set Layout *********************

	vs_descriptor_set_layout::vs_descriptor_set_layout(
		vs_device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
		: device_{device}, bindings_{bindings}
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
		for (auto kv : bindings)
		{
			setLayoutBindings.push_back(kv.second);
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
		descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(
			device.device(),
			&descriptorSetLayoutInfo,
			nullptr,
			&descriptor_set_layout_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	vs_descriptor_set_layout::~vs_descriptor_set_layout()
	{
		vkDestroyDescriptorSetLayout(device_.device(), descriptor_set_layout_, nullptr);
	}

	// *************** Descriptor Pool vs_builder *********************

	vs_descriptor_pool::vs_builder& vs_descriptor_pool::vs_builder::addPoolSize(
		VkDescriptorType descriptorType, uint32_t count)
	{
		pool_sizes_.push_back({descriptorType, count});
		return *this;
	}

	vs_descriptor_pool::vs_builder& vs_descriptor_pool::vs_builder::setPoolFlags(
		VkDescriptorPoolCreateFlags flags)
	{
		pool_flags_ = flags;
		return *this;
	}

	vs_descriptor_pool::vs_builder& vs_descriptor_pool::vs_builder::setMaxSets(uint32_t count)
	{
		max_sets_ = count;
		return *this;
	}

	std::unique_ptr<vs_descriptor_pool> vs_descriptor_pool::vs_builder::build() const
	{
		return std::make_unique<vs_descriptor_pool>(device_, max_sets_, pool_flags_, pool_sizes_);
	}

	// *************** Descriptor Pool *********************

	vs_descriptor_pool::vs_descriptor_pool(
		vs_device& device,
		uint32_t maxSets,
		VkDescriptorPoolCreateFlags poolFlags,
		const std::vector<VkDescriptorPoolSize>& poolSizes)
		: device_{device}
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		descriptorPoolInfo.flags = poolFlags;

		if (vkCreateDescriptorPool(device.device(), &descriptorPoolInfo, nullptr, &descriptor_pool_) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	vs_descriptor_pool::~vs_descriptor_pool()
	{
		vkDestroyDescriptorPool(device_.device(), descriptor_pool_, nullptr);
	}

	bool vs_descriptor_pool::allocateDescriptor(
		const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptor_pool_;
		allocInfo.pSetLayouts = &descriptorSetLayout;
		allocInfo.descriptorSetCount = 1;

		// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
		// a new pool whenever an old pool fills up. But this is beyond our current scope
		// vkguide.dev/docs/extra-chapter/abstracting_descriptors/
		if (vkAllocateDescriptorSets(device_.device(), &allocInfo, &descriptor) != VK_SUCCESS)
		{
			throw std::runtime_error("allocate descriptor sets failed in descriptor_pool");
			return false;
		}
		return true;
	}

	void vs_descriptor_pool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
	{
		vkFreeDescriptorSets(
			device_.device(),
			descriptor_pool_,
			static_cast<uint32_t>(descriptors.size()),
			descriptors.data());
	}

	void vs_descriptor_pool::resetPool()
	{
		vkResetDescriptorPool(device_.device(), descriptor_pool_, 0);
	}

	// *************** Descriptor Writer *********************

	vs_descriptor_writer::vs_descriptor_writer(vs_descriptor_set_layout& setLayout, vs_descriptor_pool& pool)
		: set_layout_{setLayout}, pool_{pool}
	{
	}

	vs_descriptor_writer& vs_descriptor_writer::writeBuffer(
		uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
	{
		assert(set_layout_.bindings_.count(binding) == 1 && "Layout does not contain specified binding");

		auto& bindingDescription = set_layout_.bindings_[binding];

		assert(
			bindingDescription.descriptorCount == 1 &&
			"Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = bufferInfo;
		write.descriptorCount = 1;

		writes_.push_back(write);
		return *this;
	}

	vs_descriptor_writer& vs_descriptor_writer::writeImage(
		uint32_t binding, VkDescriptorImageInfo* imageInfo)
	{
		assert(set_layout_.bindings_.count(binding) == 1 && "Layout does not contain specified binding");

		auto& bindingDescription = set_layout_.bindings_[binding];

		assert(
			bindingDescription.descriptorCount == 1 &&
			"Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = imageInfo;
		write.descriptorCount = 1;

		writes_.push_back(write);
		return *this;
	}

	bool vs_descriptor_writer::build(VkDescriptorSet& set)
	{
		bool success = pool_.allocateDescriptor(set_layout_.getDescriptorSetLayout(), set);
		if (!success)
		{
			return false;
		}
		overwrite(set);
		return true;
	}

	void vs_descriptor_writer::overwrite(VkDescriptorSet& set)
	{
		for (auto& write : writes_)
		{
			write.dstSet = set;
		}
		vkUpdateDescriptorSets(pool_.device_.device(), writes_.size(), writes_.data(), 0, nullptr);
	}
}
