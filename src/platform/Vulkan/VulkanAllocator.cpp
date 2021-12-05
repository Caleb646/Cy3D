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


	void VulkanAllocator::createBuffer(buffer_info_type& buffInfo, buffer_type& buffer, buffer_memory_type& allocation, offsets_type offsets)
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, vmaCreateBuffer(_allocator, &buffInfo.bufferInfo, &buffInfo.allocCreateInfo, &buffer, &allocation, &buffInfo.allocInfo) == VK_SUCCESS, "Failed to create buffer.");

		if (offsets.size() > 0)
		{
			if (buffInfo.needStagingBuffer)
			{
				buffer_type stagingBuffer;
				buffer_memory_type stagingMemory;
				buffer_info_type stagingBuffInfo = buffer_info_type::createDefaultStagingBufferInfo(buffInfo.bufferInfo.size);
				vmaCreateBuffer(_allocator, &stagingBuffInfo.bufferInfo, &stagingBuffInfo.allocCreateInfo, &stagingBuffer, &stagingMemory, &stagingBuffInfo.allocInfo);
				fillBuffer(stagingBuffInfo.allocInfo, stagingMemory, stagingBuffInfo.bufferInfo.size, offsets);
				copyBuffer(stagingBuffer, buffer, stagingBuffInfo.bufferInfo.size);
				//cleanup staging buffer
				destroyBuffer(stagingBuffer, stagingMemory);
			}
			else
			{
				fillBuffer(buffInfo.allocInfo, allocation, buffInfo.bufferInfo.size, offsets);
			}
		}
	}

	void VulkanAllocator::fillBuffer(VmaAllocationInfo allocInfo, buffer_memory_type& allocation, VkDeviceSize bufferSize, offsets_type offsets, bool unmap)
	{
		//TODO if allocation is not visible to the host create a staging buffer and transfer data to it.
		//like https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/memory_mapping.html
		ASSERT_ERROR(DEFAULT_LOGGABLE, isCPUVisible(allocInfo) == true, "Trying to map device memory.");

		void* dataDestination;
		vmaMapMemory(_allocator, allocation, &dataDestination);

		std::byte* ptr = static_cast<std::byte*>(dataDestination);
		for (auto& info : offsets)
		{
			ptr += info.offset;
			memcpy(ptr, info.data, static_cast<std::size_t>(info.bufferSize));
		}

		//if (unmap)
		//{
		//want to unmap staging buffers.
		vmaUnmapMemory(_allocator, allocation);
		//}
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

	void VulkanAllocator::destroyBuffer(VkBuffer& buffer, VmaAllocation& allocation)
	{
		vmaDestroyBuffer(_allocator, buffer, allocation);
	}

	bool VulkanAllocator::isCPUVisible(VmaAllocationInfo allocInfo)
	{
		VkMemoryPropertyFlags memFlags;
		vmaGetMemoryTypeProperties(_allocator, allocInfo.memoryType, &memFlags);
		return (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
	}

	void VulkanAllocator::createImage(image_info_type& imageInfo, image_type& image, image_memory_type& allocation, void* data)
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, vmaCreateImage(_allocator, &imageInfo.imageCreateInfo, &imageInfo.allocCreateInfo, &image, &allocation, &imageInfo.allocInfo) == VK_SUCCESS, "Failed to create image.");
	}

	void VulkanAllocator::copyBufferToImage(buffer_type& srcBuffer, image_type& dstImage, const image_info_type& imageInfo)
	{
		VkCommandBuffer commandBuffer = cyContext.getDevice()->beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = imageInfo.imageCreateInfo.extent;

		vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		cyContext.getDevice()->endSingleTimeCommands(commandBuffer);
	}

	void VulkanAllocator::destroyImage(image_type& image, image_memory_type& allocation)
	{
		vmaDestroyImage(_allocator, image, allocation);
	}
}
