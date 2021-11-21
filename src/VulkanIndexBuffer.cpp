#include "pch.h"

#include "VulkanIndexBuffer.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"


namespace cy3d
{
	VulkanIndexBuffer::VulkanIndexBuffer(VulkanContext& context, VkDeviceSize buffSize, void* data) : cyContext(context), _bufferSize(buffSize)
	{
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		//create and copy data to the staging buffer
		cyContext.getDevice()->createBuffer(BufferCreationAllocationInfo::createDefaultStagingBufferInfo(_bufferSize), stagingBuffer, stagingBufferMemory, data);

		//create the index buffer and map its memory
		cyContext.getDevice()->createBuffer(BufferCreationAllocationInfo::createDefaultDeviceOnlyIndexBufferInfo(_bufferSize), indexBuffer, indexBufferMemory);

		//transfer the data from the staging buffer to the vertex buffer.
		cyContext.getDevice()->copyBuffer(stagingBuffer, indexBuffer, _bufferSize);

		//cleanup the staging the buffer.
		vkDestroyBuffer(cyContext.getDevice()->device(), stagingBuffer, nullptr);
		vkFreeMemory(cyContext.getDevice()->device(), stagingBufferMemory, nullptr);

		mapped = true;
	}

	/**
	 * @brief
	 * @note as long as the object this class belongs to doesnt outlive the Device class the
	 * default destructor will work.
	*/
	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		cleanup();
	}

	/**
	 * @brief Creates a buffer
	 * @param data pass std::vector<vertex>.data() or equivalent.
	 * @param size is the size of the first element multiplied by the number of elements.
	*/
	void VulkanIndexBuffer::setData(void* data, VkDeviceSize size)
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, false, "Method is not implemented.");
	}

	/**
	 * @brief Creates a buffer
	 * @param data pass std::vector<vertex>.data() or equivalent.
	 * @param size is the size of the first element multiplied by the number of elements.
	 * @param bufferInfo
	*/
	void VulkanIndexBuffer::setData(void* data, BufferCreationAllocationInfo bufferInfo)
	{
		/**
		 * Before adding data to the Vertex Buffer first check if the indexBuffer and indexBufferMemory are nullptr.
		 * If not that means their current memory needs to be freed before more is added.
		*/
		ASSERT_ERROR(DEFAULT_LOGGABLE, false, "Method is not implemented.");
	}

	/**
	 * @brief Creates buffer with a section of memory mapped out but no data in it.
	*/
	void VulkanIndexBuffer::create(BufferCreationAllocationInfo bufferInfo)
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, false, "Method is not implemented.");
	}

	void VulkanIndexBuffer::copyTo(VulkanIndexBuffer* destination, VkDeviceSize size)
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, false, "Method is not implemented.");
	}

	/**
	 * @brief Should not be called from outside this class because it will cause the same
	 * buffer and memory buffer to be destroyed twice. Once by the cleanup method and once by the
	 * destructor when it goes out of scope.
	*/
	void VulkanIndexBuffer::cleanup()
	{
		if (mapped)
		{
			vkDestroyBuffer(cyContext.getDevice()->device(), indexBuffer, nullptr);
			vkFreeMemory(cyContext.getDevice()->device(), indexBufferMemory, nullptr);
		}
	}
}
