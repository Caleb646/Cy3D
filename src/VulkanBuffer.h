#pragma once
#include "pch.h"

#include <Logi/Logi.h>

#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanAllocator.h"


namespace cy3d
{
	class VulkanBuffer
	{

    public:
        using element_count_type = uint32_t;
        using instance_count_type = uint32_t;
        using buffer_size_type = VkDeviceSize;
        using offset_type = VkDeviceSize;
        using buffer_type = VkBuffer;
        using buffer_memory_type = VmaAllocation;

    private:
        buffer_type _buffer{ nullptr };
        buffer_memory_type _bufferMemory{ nullptr };
        VulkanContext& cyContext;

        element_count_type _count;
        instance_count_type _instanceCount;
        buffer_size_type _bufferSize;
        offset_type _offset;

        //tracks if the buffer and buffer memory have been mapped
        bool _mapped{ false };

    public:
        VulkanBuffer(VulkanContext& context, BufferCreationAllocationInfo bufferInfo)
            :
            cyContext(context), _count(0), _instanceCount(1), _bufferSize(bufferInfo.bufferInfo.size), _offset(0)
        {
            cyContext.getAllocator()->createBuffer(bufferInfo, _buffer, _bufferMemory);
        }

        template<typename T>
        VulkanBuffer(VulkanContext& context, BufferCreationAllocationInfo bufferInfo, T* data)
            :
            cyContext(context), _count(bufferInfo.bufferInfo.size / sizeof(T)), _instanceCount(1), _bufferSize(bufferInfo.bufferInfo.size), _offset(0)
        {
            cyContext.getAllocator()->createBuffer(bufferInfo, _buffer, _bufferMemory, { {data, bufferSize(), 0} });
        }

        template<typename T>
        VulkanBuffer(VulkanContext& context, buffer_size_type buffSize, T* data, VkBufferUsageFlags usage) 
            : 
            cyContext(context), _count(buffSize / sizeof(T)), _instanceCount(1), _bufferSize(buffSize), _offset(0)
        {
            buffer_type stagingBuffer{};
            buffer_memory_type stagingBufferMemory{};
            //create and copy data to the staging buffer
            cyContext.getAllocator()->createBuffer(BufferCreationAllocationInfo::createDefaultStagingBufferInfo(bufferSize()), stagingBuffer, stagingBufferMemory, data);

            //create the index buffer and map its memory                                                                        //ensure that  transfer dst bit is set.
            cyContext.getAllocator()->createBuffer(BufferCreationAllocationInfo::createGPUOnlyBufferInfo(bufferSize(), usage), _buffer, _bufferMemory);

            //transfer the data from the staging buffer to the vertex buffer.
            cyContext.getAllocator()->copyBuffer(stagingBuffer, _buffer, bufferSize());

            //cleanup the staging the buffer.
            cyContext.getAllocator()->destroyBuffer(stagingBuffer, stagingBufferMemory);

            _mapped = true;
        }

        template<typename V, typename I>
        VulkanBuffer(VulkanContext& context, buffer_size_type vBuffSize, V* vData, buffer_size_type iBuffSize, I* iData)
            :
            cyContext(context), _count(iBuffSize / sizeof(I)), _instanceCount(1), _bufferSize(vBuffSize + iBuffSize), _offset(vBuffSize)
        {
            buffer_type stagingBuffer{};
            buffer_memory_type stagingBufferMemory{};
            //create and copy data to the staging buffer
            VmaAllocationInfo allocInfo = cyContext.getAllocator()->createBuffer(BufferCreationAllocationInfo::createDefaultStagingBufferInfo(bufferSize()), stagingBuffer, stagingBufferMemory);

            std::vector<OffsetsInfo> offsetInfo = {
                {vData, vBuffSize, 0},
                {iData, iBuffSize, vBuffSize}
            };
            cyContext.getAllocator()->fillBuffer(allocInfo, stagingBufferMemory, bufferSize(), offsetInfo);

            //create the index buffer and map its memory
            cyContext.getAllocator()->createBuffer(BufferCreationAllocationInfo::createDefaultVertexIndexSharedBufferInfo(bufferSize()), _buffer, _bufferMemory);

            //transfer the data from the staging buffer to the vertex buffer.
            cyContext.getAllocator()->copyBuffer(stagingBuffer, _buffer, bufferSize());

            //cleanup the staging the buffer.
            cyContext.getAllocator()->destroyBuffer(stagingBuffer, stagingBufferMemory);

            _mapped = true;
        }

        VulkanBuffer(VulkanBuffer&& other) noexcept : cyContext(other.cyContext), _buffer(std::move(_buffer)), _bufferMemory(std::move(other._bufferMemory)),
            _count(other._count), _instanceCount(other._instanceCount), _bufferSize(other._bufferSize), _offset(other._offset) {}

        ~VulkanBuffer();

        //cannot be created without a context.
        VulkanBuffer() = delete;
        //cannot be copied or reassigned.
        VulkanBuffer(const VulkanBuffer&) = delete;
        //VulkanBuffer(VulkanBuffer&&) = delete;
        VulkanBuffer& operator=(const VulkanBuffer&) = delete;

        void setData(void* data, buffer_size_type size, offset_type offset);
        void setData(void* data, BufferCreationAllocationInfo bufferInfo);

        void create(BufferCreationAllocationInfo bufferInfo);
        void copyTo(VulkanBuffer* destination, VkDeviceSize size);

        element_count_type count() { return _count; }
        instance_count_type instanceCount() { return _instanceCount; }
        buffer_size_type bufferSize() { return _bufferSize; }
        offset_type offset() { return _offset; }

        buffer_type getBuffer()
        {
            ASSERT_ERROR(DEFAULT_LOGGABLE, _buffer != nullptr, "Data has not been set. Buffer is null.");
            return _buffer;
        }

    private:
        void cleanup();
	};
}


