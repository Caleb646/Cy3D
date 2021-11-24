#pragma once
#include "pch.h"

#include "vma/vk_mem_alloc.h"

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


	class VulkanAllocator
	{
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

		VmaAllocationInfo createBuffer(BufferCreationAllocationInfo buffInfo, VkBuffer& buffer, VmaAllocation& allocation, const std::vector<OffsetsInfo>& offsets = {});
		void fillBuffer(VmaAllocationInfo allocInfo, VmaAllocation& allocation, VkDeviceSize bufferSize, const std::vector<OffsetsInfo>& offsets, bool unmap = true);
		void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);
		void destroyBuffer(VkBuffer& buffer, VmaAllocation& allocation);

		bool isCPUVisible(VmaAllocationInfo allocInfo);
	};
}


