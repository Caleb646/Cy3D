#include "pch.h"
#include "VulkanBuffer.h"

namespace cy3d
{
	VulkanBuffer::~VulkanBuffer()
	{
		cleanup();
	}

	void VulkanBuffer::cleanup()
	{
		if (_buffer != nullptr && _bufferMemory != nullptr)
		{
			cyContext.getAllocator()->destroyBuffer(_buffer, _bufferMemory);
		}
	}
}
