#pragma once
#include "pch.h"

#include "vma/vk_mem_alloc.h"

#include "Vulkan.h"
#include "Fwd.hpp"

namespace cy3d
{
	struct OffsetsInfo
	{
		void* data;
		VkDeviceSize bufferSize;
		VkDeviceSize offset;
	};

	struct BufferCreationAllocationInfo
	{

		VkBufferCreateInfo bufferInfo{};
		VmaAllocationCreateInfo allocCreateInfo{};
		VmaAllocationInfo allocInfo{};
		bool needStagingBuffer{ false };

		/*
		* Possible Usage bits:
		* VK_BUFFER_USAGE_TRANSFER_SRC_BIT -> Buffer can be used as source in a memory transfer operation.
		* VK_BUFFER_USAGE_TRANSFER_DST_BIT -> Buffer can be used as destination in a memory transfer operation.
		* VK_BUFFER_USAGE_VERTEX_BUFFER_BIT -> Buffer is used for vertexs.
		*/

		static BufferCreationAllocationInfo createDefaultUniformBuffer(VkDeviceSize bufferSize)
		{
			return createCPUOnlyBufferInfo(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		}

		static BufferCreationAllocationInfo createDefaultStagingBufferInfo(VkDeviceSize bufferSize)
		{
			//return createGPUCPUCoherentBufferInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

			return createCPUOnlyBufferInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		}


		/**
		 * @brief Intended to work in tandem with a staging buffer so the first usage flag is set as
		 * VK_BUFFER_USAGE_TRANSFER_DST_BIT.
		*/
		static BufferCreationAllocationInfo createDefaultGPUOnlyIndexBufferInfo(VkDeviceSize bufferSize)
		{
			return createGPUOnlyBufferInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		}

		static BufferCreationAllocationInfo createDefaultVertexIndexSharedBufferInfo(VkDeviceSize bufferSize)
		{
			return createGPUOnlyBufferInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		}

		/**
		 * @brief Intended to work in tandem with a staging buffer so the first usage flag is set as
		 * VK_BUFFER_USAGE_TRANSFER_DST_BIT.
		*/
		static BufferCreationAllocationInfo createDefaultGPUOnlyVertexBufferInfo(VkDeviceSize bufferSize)
		{
			return createGPUOnlyBufferInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		}

		/**
		 * @brief Will create a vertex buffer info that can be written to by the CPU.
		*/
		static BufferCreationAllocationInfo createDefaultVertexBufferInfo(VkDeviceSize bufferSize)
		{
			return createGPUCPUCoherentBufferInfo(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		}

		/**
		 * @brief Specify a buffer setup that can be accessed by the CPU and GPU.
		 * 
		 * A memory type is chosen that has all the required flags and as many preferred flags set as possible.
		*/
		static BufferCreationAllocationInfo createGPUCPUCoherentBufferInfo(VkDeviceSize bufferSize, VkBufferUsageFlags usage)
		{

			BufferCreationAllocationInfo buffInfo = createBufferInfo(bufferSize, usage);
			buffInfo.allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			buffInfo.allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			return buffInfo;
		}

		/**
		 * @brief Specify a buffer setup that is intended to only be accessible by the GPU.
		*/
		static BufferCreationAllocationInfo createGPUOnlyBufferInfo(VkDeviceSize bufferSize, VkBufferUsageFlags usage)
		{

			BufferCreationAllocationInfo buffInfo = createBufferInfo(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
			buffInfo.allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			buffInfo.allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			buffInfo.needStagingBuffer = true;

			return buffInfo;
		}

		/**
		 * @brief Specify a buffer setup that is intended to only be accessible by the GPU.
		*/
		static BufferCreationAllocationInfo createCPUOnlyBufferInfo(VkDeviceSize bufferSize, VkBufferUsageFlags usage)
		{

			BufferCreationAllocationInfo buffInfo = createBufferInfo(bufferSize, usage);
			buffInfo.allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
			//buffInfo.allocCreateInfo.requiredFlags = VK_MEMORY_;

			return buffInfo;
		}

		static BufferCreationAllocationInfo createBufferInfo(VkDeviceSize bufferSize, VkBufferUsageFlags usage)
		{
			VkBufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = bufferSize;
			info.usage = usage;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocInfo{};

			return BufferCreationAllocationInfo{ info, allocInfo };
		}
	};


	struct ImageInfo
	{
		VkFormat format{};
		VkImageTiling tiling{};
		VkImageUsageFlags usage{};
		VkImageAspectFlags aspectFlags{};
		uint32_t width{};
		uint32_t height{};
		VkDeviceSize imageSize{};
	};

	struct ImageCreationAllocationInfo
	{
		VkImageCreateInfo imageCreateInfo{};
		ImageInfo imageInfo{};
		VmaAllocationCreateInfo allocCreateInfo{};
		VmaAllocationInfo allocInfo{};

		static ImageCreationAllocationInfo createDefaultImageInfo(ImageInfo info)
		{
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = info.width;
			imageInfo.extent.height = info.height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = info.format;
			imageInfo.tiling = info.tiling;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = info.usage;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.flags = 0; // Optional


			VmaAllocationCreateInfo allocCreateInfo{};
			allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			//allocCreateInfo.requiredFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

			VmaAllocationInfo allocInfo{};

			return ImageCreationAllocationInfo{ imageInfo, info, allocCreateInfo, allocInfo };
		}
	};


	class VulkanAllocator
	{
	public:
		using buffer_type = VkBuffer;
		using buffer_memory_type = VmaAllocation;
		using image_type = VkImage;
		using image_memory_type = VmaAllocation;
		using buffer_info_type = BufferCreationAllocationInfo;
		using image_info_type = ImageCreationAllocationInfo;
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

		void createBuffer(buffer_info_type& buffInfo, buffer_type& buffer, buffer_memory_type& allocation, offsets_type offsets = {});
		void fillBuffer(VmaAllocationInfo allocInfo, buffer_memory_type& allocation, VkDeviceSize bufferSize, offsets_type offsets, bool unmap = true);
		void copyBuffer(buffer_type& srcBuffer, buffer_type& dstBuffer, VkDeviceSize size);
		void destroyBuffer(buffer_type& buffer, buffer_memory_type& allocation);

		void createImage(image_info_type& buffInfo, image_type& buffer, image_memory_type& allocation, void* data = nullptr);
		void copyBufferToImage(buffer_type& srcBuffer, image_type& dstImage, const image_info_type& imageInfo);
		void destroyImage(image_type& image, image_memory_type& allocation);

		bool isCPUVisible(VmaAllocationInfo allocInfo);
	};
}


