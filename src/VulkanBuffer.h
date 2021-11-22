#pragma once
#include "pch.h"

#include <Logi/Logi.h>

#include "VulkanDevice.h"
#include "VulkanContext.h"


namespace cy3d
{
	class VulkanBuffer
	{
    private:
        using element_count_type = uint32_t;
        using instance_count_type = uint32_t;
        using buffer_size_type = VkDeviceSize;
        using offset_type = VkDeviceSize;

        VkBuffer _buffer{ nullptr };
        VkDeviceMemory _bufferMemory{ nullptr };
        VulkanContext& cyContext;

        element_count_type _count;
        instance_count_type _instanceCount;
        buffer_size_type _bufferSize;
        offset_type _offset;

        //tracks if the buffer and buffer memory have been mapped
        bool _mapped{ false };

    public:

        template<typename T>
        VulkanBuffer(VulkanContext& context, buffer_size_type buffSize, T* data) 
            : 
            cyContext(context), _count(buffSize / sizeof(T)), _instanceCount(1), _bufferSize(buffSize), _offset(0)
        {
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            //create and copy data to the staging buffer
            cyContext.getDevice()->createBuffer(BufferCreationAllocationInfo::createDefaultStagingBufferInfo(bufferSize()), stagingBuffer, stagingBufferMemory, data);

            //create the index buffer and map its memory
            cyContext.getDevice()->createBuffer(BufferCreationAllocationInfo::createDefaultDeviceOnlyIndexBufferInfo(bufferSize()), _buffer, _bufferMemory);

            //transfer the data from the staging buffer to the vertex buffer.
            cyContext.getDevice()->copyBuffer(stagingBuffer, _buffer, bufferSize());

            //cleanup the staging the buffer.
            vkDestroyBuffer(cyContext.getDevice()->device(), stagingBuffer, nullptr);
            vkFreeMemory(cyContext.getDevice()->device(), stagingBufferMemory, nullptr);

            _mapped = true;
        }

        template<typename V, typename I>
        VulkanBuffer(VulkanContext& context, buffer_size_type vBuffSize, V* vData, buffer_size_type iBuffSize, I* iData)
            :
            cyContext(context), _count(iBuffSize / sizeof(I)), _instanceCount(1), _bufferSize(vBuffSize + iBuffSize), _offset(vBuffSize)
        {
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            //create and copy data to the staging buffer
            cyContext.getDevice()->createBuffer(BufferCreationAllocationInfo::createDefaultStagingBufferInfo(bufferSize()), stagingBuffer, stagingBufferMemory);

            //TODO HAVE a memory leak its prob due to this class.

            std::vector<OffsetsInfo> offsetInfo = {
                {vData, vBuffSize, 0},
                {iData, iBuffSize, vBuffSize}
            };
            cyContext.getDevice()->fillBuffer(stagingBufferMemory, bufferSize(), offsetInfo);

            //create the index buffer and map its memory
            cyContext.getDevice()->createBuffer(BufferCreationAllocationInfo::createDefaultVertexIndexSharedBufferInfo(bufferSize()), _buffer, _bufferMemory);

            //transfer the data from the staging buffer to the vertex buffer.
            cyContext.getDevice()->copyBuffer(stagingBuffer, _buffer, bufferSize());

            //cleanup the staging the buffer.
            vkDestroyBuffer(cyContext.getDevice()->device(), stagingBuffer, nullptr);
            vkFreeMemory(cyContext.getDevice()->device(), stagingBufferMemory, nullptr);

            _mapped = true;
        }
        ~VulkanBuffer();

        //cannot be created without a context.
        VulkanBuffer() = delete;
        //cannot be copied or reassigned.
        VulkanBuffer(const VulkanBuffer&) = delete;
        VulkanBuffer(VulkanBuffer&&) = delete;
        VulkanBuffer& operator=(const VulkanBuffer&) = delete;

        void setData(void* data, buffer_size_type size, offset_type offset);
        void setData(void* data, BufferCreationAllocationInfo bufferInfo);

        void create(BufferCreationAllocationInfo bufferInfo);
        void copyTo(VulkanBuffer* destination, VkDeviceSize size);

        element_count_type count() { return _count; }
        instance_count_type instanceCount() { return _instanceCount; }
        buffer_size_type bufferSize() { return _bufferSize; }
        offset_type offset() { return _offset; }

        VkBuffer getBuffer()
        {
            ASSERT_ERROR(DEFAULT_LOGGABLE, _buffer != nullptr, "Data has not been set. Buffer is null.");
            return _buffer;
        }

    private:
        void cleanup();
	};
}


