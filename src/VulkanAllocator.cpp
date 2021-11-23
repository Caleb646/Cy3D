#include "pch.h"

#include "VulkanAllocator.h"

#include <Logi/Logi.h>

#include "VulkanContext.h"
#include "VulkanDevice.h"



namespace cy3d
{
	VulkanAllocator::VulkanAllocator(VulkanContext& context) : cyContext(context)
	{
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.vulkanApiVersion = CY_VK_API_VERSION;
		allocatorInfo.physicalDevice = cyContext.getDevice()->physicalDevice();
		allocatorInfo.device = cyContext.getDevice()->device();
		allocatorInfo.instance = cyContext.getDevice()->instance();
		vmaCreateAllocator(&allocatorInfo, &_allocator);
	}

	VulkanAllocator::~VulkanAllocator()
	{
		vmaDestroyAllocator(_allocator);
	}


	VmaAllocationInfo VulkanAllocator::createBuffer(BufferCreationAllocationInfo buffInfo, VkBuffer& buffer, VmaAllocation& allocation, void* data)
	{
		VmaAllocationInfo allocInfo;
		ASSERT_ERROR(DEFAULT_LOGGABLE, vmaCreateBuffer(_allocator, &buffInfo.bufferInfo, &buffInfo.allocCreateInfo, &buffer, &allocation, &allocInfo) == VK_SUCCESS, "Failed to create buffer.");

		if (data != nullptr)
		{
			fillBuffer(allocInfo, allocation, buffInfo.bufferInfo.size, { {data, buffInfo.bufferInfo.size, 0} });
		}

		return allocInfo;
	}

	void VulkanAllocator::destroyBuffer(VkBuffer& buffer, VmaAllocation& allocation)
	{
		vmaDestroyBuffer(_allocator, buffer, allocation);
	}

	void VulkanAllocator::fillBuffer(VmaAllocationInfo allocInfo, VmaAllocation& allocation, VkDeviceSize bufferSize, const std::vector<OffsetsInfo>& offsets, bool unmap)
	{
		//TODO if allocation is not visible to the host create a staging buffer and transfer data to it.
		ASSERT_ERROR(DEFAULT_LOGGABLE, isHostVisible(allocInfo) == true, "Trying to transfer data to device memory.");

		void* dataDestination;
		vmaMapMemory(_allocator, allocation, &dataDestination);

		std::byte* ptr = static_cast<std::byte*>(dataDestination);
		for (auto& info : offsets)
		{
			ptr += info.offset;
			memcpy(ptr, info.data, static_cast<std::size_t>(info.bufferSize));
		}

		if (unmap)
		{
			vmaUnmapMemory(_allocator, allocation);
		}
	}

	void VulkanAllocator::copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = cyContext.getDevice()->beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;  // Optional
		copyRegion.dstOffset = 0;  // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		cyContext.getDevice()->endSingleTimeCommands(commandBuffer);
	}

	bool VulkanAllocator::isHostVisible(VmaAllocationInfo allocInfo)
	{
		VkMemoryPropertyFlags memFlags;
		vmaGetMemoryTypeProperties(_allocator, allocInfo.memoryType, &memFlags);
		return (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
	}
}
