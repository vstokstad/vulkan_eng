/*
 * Encapsulates a vulkan buffer_
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

#include "vs_buffer.h"
// std
#include <cassert>
#include <cstring>

namespace vs
{
	/**
	 * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
	 *
	 * @param instanceSize The size of an instance
	 * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
	 * minUniformBufferOffsetAlignment)
	 *
	 * @return VkResult of the buffer mapping call
	 */
	VkDeviceSize vs_buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
	{
		if (minOffsetAlignment > 0)
		{
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		}
		return instanceSize;
	}

	vs_buffer::vs_buffer(
		vs_device& device,
		VkDeviceSize instanceSize,
		uint32_t instanceCount,
		VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryPropertyFlags,
		VkDeviceSize minOffsetAlignment)
		: device_{device},
		  instance_count_{instanceCount},
		  instance_size_{instanceSize},
		  usage_flags_{usageFlags},
		  memory_property_flags_{memoryPropertyFlags}
	{
		alignment_size_ = getAlignment(instanceSize, minOffsetAlignment);
		buffer_size_ = alignment_size_ * instanceCount;
		device.createBuffer(buffer_size_, usageFlags, memoryPropertyFlags, buffer_, memory_);
	}

	vs_buffer::~vs_buffer()
	{
		unmap();
		vkDestroyBuffer(device_.device(), buffer_, nullptr);
		vkFreeMemory(device_.device(), memory_, nullptr);
	}

	/**
	 * Map a memory_ range of this buffer_. If successful, mapped_ points to the specified buffer_ range.
	 *
	 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
	 * buffer range.
	 * @param offset (Optional) Byte offset from beginning
	 *
	 * @return VkResult of the buffer mapping call
	 */
	VkResult vs_buffer::map(VkDeviceSize size, VkDeviceSize offset)
	{
		assert(buffer_ && memory_ && "Called map on buffer_ before create");
		return vkMapMemory(device_.device(), memory_, offset, size, 0, &mapped_);
	}

	/**
	 * Unmap a mapped_ memory_ range
	 *
	 * @note Does not return a result as vkUnmapMemory can't fail
	 */
	void vs_buffer::unmap()
	{
		if (mapped_)
		{
			vkUnmapMemory(device_.device(), memory_);
			mapped_ = nullptr;
		}
	}

	/**
	 * Copies the specified data to the mapped_ buffer_. Default value writes whole buffer_ range
	 *
	 * @param data Pointer to the data to copy
	 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
	 * range.
	 * @param offset (Optional) Byte offset from beginning of mapped region
	 *
	 */
	void vs_buffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		assert(mapped_ && "Cannot copy to unmapped buffer_");

		if (size == VK_WHOLE_SIZE)
		{
			memcpy(mapped_, data, buffer_size_);
		}
		else
		{
			char* memOffset = (char*)mapped_;
			memOffset += offset;
			memcpy(memOffset, data, size);
		}
	}

	/**
	 * Flush a memory_ range of the buffer_ to make it visible to the device
	 *
	 * @note Only required for non-coherent memory
	 *
	 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
	 * complete buffer range.
	 * @param offset (Optional) Byte offset from beginning
	 *
	 * @return VkResult of the flush call
	 */
	VkResult vs_buffer::flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory_;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(device_.device(), 1, &mappedRange);
	}

	/**
	 * Invalidate a memory_ range of the buffer_ to make it visible to the host
	 *
	 * @note Only required for non-coherent memory
	 *
	 * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
	 * the complete buffer range.
	 * @param offset (Optional) Byte offset from beginning
	 *
	 * @return VkResult of the invalidate call
	 */
	VkResult vs_buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory_;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(device_.device(), 1, &mappedRange);
	}

	/**
	 * Create a buffer_ info descriptor
	 *
	 * @param size (Optional) Size of the memory range of the descriptor
	 * @param offset (Optional) Byte offset from beginning
	 *
	 * @return VkDescriptorBufferInfo of specified offset and range
	 */
	VkDescriptorBufferInfo vs_buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset)
	{
		return VkDescriptorBufferInfo{
			buffer_,
			offset,
			size,
		};
	}

	/**
	 * Copies "instance_size_" bytes of data to the mapped_ buffer_ at an offset of index * alignment_size_
	 *
	 * @param data Pointer to the data to copy
	 * @param index Used in offset calculation
	 *
	 */
	void vs_buffer::writeToIndex(void* data, int index)
	{
		writeToBuffer(data, instance_size_, index * alignment_size_);
	}

	/**
	 *  Flush the memory_ range at index * alignment_size_ of the buffer_ to make it visible to the device
	 *
	 * @param index Used in offset calculation
	 *
	 */
	VkResult vs_buffer::flushIndex(int index) { return flush(alignment_size_, index * alignment_size_); }

	/**
	 * Create a buffer_ info descriptor
	 *
	 * @param index Specifies the region given by index * alignmentSize
	 *
	 * @return VkDescriptorBufferInfo for instance at index
	 */
	VkDescriptorBufferInfo vs_buffer::descriptorInfoForIndex(int index)
	{
		return descriptorInfo(alignment_size_, index * alignment_size_);
	}

	/**
	 * Invalidate a memory_ range of the buffer_ to make it visible to the host
	 *
	 * @note Only required for non-coherent memory
	 *
	 * @param index Specifies the region to invalidate: index * alignmentSize
	 *
	 * @return VkResult of the invalidate call
	 */
	VkResult vs_buffer::invalidateIndex(int index)
	{
		return invalidate(alignment_size_, index * alignment_size_);
	}
} // namespace lve
