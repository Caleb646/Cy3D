#pragma once
#include "pch.h"
#include "vma/vk_mem_alloc.h"


namespace cy3d
{
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

	struct ImageCreateInfo
	{
		VkImageCreateInfo imageCreateInfo{};
		ImageInfo imageInfo{};
		VmaAllocationCreateInfo allocCreateInfo{};
		VmaAllocationInfo allocInfo{};

		static ImageCreateInfo createDefaultImageInfo(ImageInfo info)
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

			return ImageCreateInfo{ imageInfo, info, allocCreateInfo, allocInfo };
		}
	};

	struct BufferCreateInfo
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

		static BufferCreateInfo createUBOInfo(VkDeviceSize bufferSize)
		{
			return createCPUOnlyBufferInfo(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		}

		static BufferCreateInfo createDefaultStagingBufferInfo(VkDeviceSize bufferSize)
		{
			//return createGPUCPUCoherentBufferInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

			return createCPUOnlyBufferInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		}


		/**
		 * @brief Intended to work in tandem with a staging buffer so the first usage flag is set as
		 * VK_BUFFER_USAGE_TRANSFER_DST_BIT.
		*/
		static BufferCreateInfo createDefaultGPUOnlyIndexBufferInfo(VkDeviceSize bufferSize)
		{
			return createGPUOnlyBufferInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		}

		static BufferCreateInfo createDefaultVertexIndexSharedBufferInfo(VkDeviceSize bufferSize)
		{
			return createGPUOnlyBufferInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		}

		/**
		 * @brief Intended to work in tandem with a staging buffer so the first usage flag is set as
		 * VK_BUFFER_USAGE_TRANSFER_DST_BIT.
		*/
		static BufferCreateInfo createDefaultGPUOnlyVertexBufferInfo(VkDeviceSize bufferSize)
		{
			return createGPUOnlyBufferInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		}

		/**
		 * @brief Will create a vertex buffer info that can be written to by the CPU.
		*/
		static BufferCreateInfo createDefaultVertexBufferInfo(VkDeviceSize bufferSize)
		{
			return createGPUCPUCoherentBufferInfo(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		}

		/**
		 * @brief Specify a buffer setup that can be accessed by the CPU and GPU.
		 *
		 * A memory type is chosen that has all the required flags and as many preferred flags set as possible.
		*/
		static BufferCreateInfo createGPUCPUCoherentBufferInfo(VkDeviceSize bufferSize, VkBufferUsageFlags usage)
		{

			BufferCreateInfo buffInfo = createBufferInfo(bufferSize, usage);
			buffInfo.allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			buffInfo.allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			return buffInfo;
		}

		/**
		 * @brief Specify a buffer setup that is intended to only be accessible by the GPU.
		*/
		static BufferCreateInfo createGPUOnlyBufferInfo(VkDeviceSize bufferSize, VkBufferUsageFlags usage)
		{

			BufferCreateInfo buffInfo = createBufferInfo(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
			buffInfo.allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			buffInfo.allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			buffInfo.needStagingBuffer = true;

			return buffInfo;
		}

		/**
		 * @brief Specify a buffer setup that is intended to only be accessible by the GPU.
		*/
		static BufferCreateInfo createCPUOnlyBufferInfo(VkDeviceSize bufferSize, VkBufferUsageFlags usage)
		{

			BufferCreateInfo buffInfo = createBufferInfo(bufferSize, usage);
			buffInfo.allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
			//buffInfo.allocCreateInfo.requiredFlags = VK_MEMORY_;

			return buffInfo;
		}

		static BufferCreateInfo createBufferInfo(VkDeviceSize bufferSize, VkBufferUsageFlags usage)
		{
			VkBufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = bufferSize;
			info.usage = usage;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocInfo{};

			return BufferCreateInfo{ info, allocInfo };
		}
	};
}


