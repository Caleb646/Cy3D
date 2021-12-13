#pragma once
#include "pch.h"
#include "vma/vk_mem_alloc.h"

#include "../../Fwd.hpp"
#include "VulkanBufferTypes.h"
#include "Vulkan.h"


namespace cy3d
{
	struct OffsetsInfo
	{
		void* data;
		VkDeviceSize bufferSize;
		VkDeviceSize offset;
	};

	class VulkanAllocator
	{
	public:
		using buffer_type = VkBuffer;
		using buffer_memory_type = VmaAllocation;
		using image_type = VkImage;
		using image_memory_type = VmaAllocation;
		using image_info_type = ImageCreateInfo;
		using offsets_type = const std::vector<OffsetsInfo>&;

	private:
		VulkanContext& cyContext;
		VmaAllocator _allocator;

	public:
		VulkanAllocator(VulkanContext&);
		~VulkanAllocator();

		VulkanAllocator() = delete;
		VulkanAllocator(const VulkanAllocator&) = delete;
		VulkanAllocator(VulkanAllocator&&) = delete;
		VulkanAllocator& operator=(const VulkanAllocator&) = delete;

		void createBuffer(BufferCreateInfo& buffInfo, buffer_type& buffer, buffer_memory_type& allocation, offsets_type offsets = {});
		void fillBuffer(VmaAllocationInfo allocInfo, buffer_memory_type& allocation, VkDeviceSize bufferSize, offsets_type offsets, bool unmap = true);
		void copyBuffer(buffer_type& srcBuffer, buffer_type& dstBuffer, VkDeviceSize size);
		void destroyBuffer(buffer_type& buffer, buffer_memory_type& allocation);

		void createImage(image_info_type& buffInfo, image_type& buffer, image_memory_type& allocation, void* data = nullptr);
		void copyBufferToImage(buffer_type& srcBuffer, image_type& dstImage, const image_info_type& imageInfo);
		void destroyImage(image_type& image, image_memory_type& allocation);

		bool isCPUVisible(VmaAllocationInfo allocInfo);
	};
}


