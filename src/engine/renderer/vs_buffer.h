#pragma once
#include "vs_device.h"

namespace vs
{
	class vs_buffer
	{
	public:
		vs_buffer(
			vs_device& device,
			VkDeviceSize instanceSize,
			uint32_t instanceCount,
			VkBufferUsageFlags usageFlags,
			VkMemoryPropertyFlags memoryPropertyFlags,
			VkDeviceSize minOffsetAlignment = 1);
		~vs_buffer();

		vs_buffer(const vs_buffer&) = delete;
		vs_buffer& operator=(const vs_buffer&) = delete;

		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void unmap();

		void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		void writeToIndex(void* data, int index);
		VkResult flushIndex(int index);
		VkDescriptorBufferInfo descriptorInfoForIndex(int index);
		VkResult invalidateIndex(int index);

		VkBuffer getBuffer() const { return buffer_; }
		void* getMappedMemory() const { return mapped_; }
		uint32_t getInstanceCount() const { return instance_count_; }
		VkDeviceSize getInstanceSize() const { return instance_size_; }
		VkDeviceSize getAlignmentSize() const { return instance_size_; }
		VkBufferUsageFlags getUsageFlags() const { return usage_flags_; }
		VkMemoryPropertyFlags getMemoryPropertyFlags() const { return memory_property_flags_; }
		VkDeviceSize getBufferSize() const { return buffer_size_; }

	private:
		static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

		vs_device& device_;
		void* mapped_ = nullptr;
		VkBuffer buffer_ = VK_NULL_HANDLE;
		VkDeviceMemory memory_ = VK_NULL_HANDLE;

		VkDeviceSize buffer_size_;
		uint32_t instance_count_;
		VkDeviceSize instance_size_;
		VkDeviceSize alignment_size_;
		VkBufferUsageFlags usage_flags_;
		VkMemoryPropertyFlags memory_property_flags_;
	};
}
