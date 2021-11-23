#include "pch.h"
//
//#include "VulkanVertexBuffer.h"
//#include "VulkanDevice.h"
//#include "VulkanContext.h"
//#include <Logi/Logi.h>
//
//
//namespace cy3d
//{
//	VulkanVertexBuffer::VulkanVertexBuffer(VulkanContext& context, VkDeviceSize s, void* data) : cyContext(context), _bufferSize(s)
//	{
//		VkBuffer stagingBuffer;
//		VkDeviceMemory stagingBufferMemory;
//		//create and copy data to the staging buffer
//		cyContext.getDevice()->createBuffer(BufferCreationAllocationInfo::createDefaultStagingBufferInfo(_bufferSize), stagingBuffer, stagingBufferMemory, data);
//
//		//create the vertex buffer and map its memory
//		cyContext.getDevice()->createBuffer(BufferCreationAllocationInfo::createDefaultDeviceOnlyVertexBufferInfo(_bufferSize), vertexBuffer, vertexBufferMemory);
//
//		//transfer the data from the staging buffer to the vertex buffer.
//		cyContext.getDevice()->copyBuffer(stagingBuffer, vertexBuffer, _bufferSize);
//
//		//cleanup the staging the buffer.
//		vkDestroyBuffer(cyContext.getDevice()->device(), stagingBuffer, nullptr);
//		vkFreeMemory(cyContext.getDevice()->device(), stagingBufferMemory, nullptr);
//
//		mapped = true;
//	}
//
//	/**
//	 * @brief
//	 * @note as long as the object this class belongs to doesnt outlive the Device class the
//	 * default destructor will work.
//	*/
//	VulkanVertexBuffer::~VulkanVertexBuffer()
//	{
//		cleanup();
//	}
//
//	/**
//	 * @brief Creates a buffer
//	 * @param data pass std::vector<vertex>.data() or equivalent.
//	 * @param size is the size of the first element multiplied by the number of elements.
//	*/
//	void VulkanVertexBuffer::setData(void* data, VkDeviceSize size)
//	{
//		ASSERT_ERROR(DEFAULT_LOGGABLE, false, "Method is not implemented.");
//		setData(data, BufferCreationAllocationInfo::createDefaultVertexBufferInfo(size));
//	}
//
//	/**
//	 * @brief Creates a buffer
//	 * @param data pass std::vector<vertex>.data() or equivalent.
//	 * @param size is the size of the first element multiplied by the number of elements.
//	 * @param bufferInfo
//	*/
//	void VulkanVertexBuffer::setData(void* data, BufferCreationAllocationInfo bufferInfo)
//	{
//		/**
//		 * Before adding data to the Vertex Buffer first check if the vertexBuffer and vertexBufferMemory are nullptr.
//		 * If not that means their current memory needs to be freed before more is added.
//		*/
//		ASSERT_ERROR(DEFAULT_LOGGABLE, false, "Method is not implemented.");
//		//if (vertexBuffer != nullptr && vertexBufferMemory != nullptr) cleanup();
//		//create(bufferInfo);
//		//cyContext.getDevice()->fillBuffer(data, bufferInfo.bufferSize, vertexBufferMemory);
//	}
//
//	/**
//	 * @brief Creates buffer with a section of memory mapped out but no data in it. 
//	*/
//	void VulkanVertexBuffer::create(BufferCreationAllocationInfo bufferInfo)
//	{
//		ASSERT_ERROR(DEFAULT_LOGGABLE, false, "Method is not implemented.");
//		//cyContext.getDevice()->createBuffer(bufferInfo, vertexBuffer, vertexBufferMemory);
//	}
//
//	void VulkanVertexBuffer::copyTo(VulkanVertexBuffer* destination, VkDeviceSize size)
//	{
//		ASSERT_ERROR(DEFAULT_LOGGABLE, false, "Method is not implemented.");
//		///cyContext.getDevice()->copyBuffer(vertexBuffer, destination->vertexBuffer, size);
//		//cleanup();
//	}
//
//	/**
//	 * @brief Should not be called from outside this class because it will cause the same
//	 * buffer and memory buffer to be destroyed twice. Once by the cleanup method and once by the 
//	 * destructor when it goes out of scope.
//	*/
//	void VulkanVertexBuffer::cleanup()
//	{
//		if (mapped)
//		{
//			vkDestroyBuffer(cyContext.getDevice()->device(), vertexBuffer, nullptr);
//			vkFreeMemory(cyContext.getDevice()->device(), vertexBufferMemory, nullptr);
//		}
//	}
//}
