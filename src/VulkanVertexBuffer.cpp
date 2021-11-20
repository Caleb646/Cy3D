#include "pch.h"

#include "VulkanVertexBuffer.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include <Logi/Logi.h>


namespace cy3d
{
	VulkanVertexBuffer::VulkanVertexBuffer(VulkanContext& context) : cyContext(context) {}

	/**
	 * @brief
	 * @note as long as the object this class belongs to doesnt outlive the Device class the
	 * default destructor will work.
	*/
	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		//TODO buffers are not being destroyed.
		cleanup();
	}

	/**
	 * @brief Creates a buffer
	 * @param data pass std::vector<vertex>.data() or equivalent.
	 * @param size is the size of the first element multiplied by the number of elements.
	*/
	void VulkanVertexBuffer::setData(void* data, VkDeviceSize size)
	{
		/**
		 * Before adding data to the Vertex Buffer first check if the vertexBuffer and vertexBufferMemory are nullptr.
		 * If not that means their current memory needs to be freed before more is added.
		*/
		BufferCreationAllocationInfo bufferInfo
		{ 
			size, 
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};
		setData(data, size, bufferInfo);
	}

	/**
	 * @brief Creates a buffer
	 * @param data pass std::vector<vertex>.data() or equivalent.
	 * @param size is the size of the first element multiplied by the number of elements.
	 * @param bufferInfo
	*/
	void VulkanVertexBuffer::setData(void* data, VkDeviceSize size, BufferCreationAllocationInfo bufferInfo)
	{
		/**
		 * Before adding data to the Vertex Buffer first check if the vertexBuffer and vertexBufferMemory are nullptr.
		 * If not that means their current memory needs to be freed before more is added.
		*/
		if (vertexBuffer != nullptr && vertexBufferMemory != nullptr) cleanup();
		cyContext.getDevice()->createBuffer(bufferInfo, vertexBuffer, vertexBufferMemory);
		cyContext.getDevice()->fillBuffer(data, size, vertexBufferMemory);
	}

	/**
	 * @brief Should not be called from outside this class because it will cause the same
	 * buffer and memory buffer to be destroyed twice. Once by the cleanup method and once by the 
	 * destructor when it goes out of scope.
	*/
	void VulkanVertexBuffer::cleanup()
	{
		vkDestroyBuffer(cyContext.getDevice()->device(), vertexBuffer, nullptr);
		vkFreeMemory(cyContext.getDevice()->device(), vertexBufferMemory, nullptr);
	}
}
