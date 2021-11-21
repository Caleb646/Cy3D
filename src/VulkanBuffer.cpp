#include "pch.h"

#include "VulkanBuffer.h"

namespace cy3d
{
	VulkanBuffer::~VulkanBuffer()
	{
		cleanup();
	}

	void VulkanBuffer::setData(void* data, buffer_size_type size, offset_type offset)
	{

	}

	void VulkanBuffer::setData(void* data, BufferCreationAllocationInfo bufferInfo)
	{

	}

	void VulkanBuffer::cleanup()
	{
		if (_mapped)
		{
			vkDestroyBuffer(cyContext.getDevice()->device(), _buffer, nullptr);
			vkFreeMemory(cyContext.getDevice()->device(), _bufferMemory, nullptr);					
		}
	}
}
