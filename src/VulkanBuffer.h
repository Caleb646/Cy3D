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
        using buffer_type = typename VulkanAllocator::buffer_type;
        using buffer_memory_type = typename VulkanAllocator::buffer_memory_type;
        using buffer_info_type = BufferCreationAllocationInfo;
        using offsets_type = typename VulkanAllocator::offsets_type;

    private:
        buffer_type _buffer{ nullptr };
        buffer_memory_type _bufferMemory{ nullptr };
        buffer_info_type _bufferInfo;
        VulkanContext& cyContext;

        element_count_type _count;
        instance_count_type _instanceCount;
        buffer_size_type _bufferSize;
        offset_type _offset;

        //tracks if the buffer and buffer memory have been mapped
        bool _mapped{ false };

    public:
        VulkanBuffer(VulkanContext& context, buffer_info_type bufferInfo, element_count_type count)
            :
            cyContext(context), _bufferInfo(bufferInfo), _count(count), _instanceCount(1), _bufferSize(bufferInfo.bufferInfo.size), _offset(0)
        {
            cyContext.getAllocator()->createBuffer(_bufferInfo, _buffer, _bufferMemory);
        }

        template<typename T>
        VulkanBuffer(VulkanContext& context, buffer_info_type bufferInfo, T* data)
            :
            cyContext(context), _bufferInfo(bufferInfo), _count(bufferInfo.bufferInfo.size / sizeof(T)), _instanceCount(1), _bufferSize(bufferInfo.bufferInfo.size), _offset(0)
        {
            cyContext.getAllocator()->createBuffer(_bufferInfo, _buffer, _bufferMemory, { {data, _bufferSize, 0} });
        }

        //template<typename T>
        //VulkanBuffer(VulkanContext& context, buffer_size_type buffSize, T* data, VkBufferUsageFlags usage) 
        //    : 
        //    cyContext(context), _count(buffSize / sizeof(T)), _instanceCount(1), _bufferSize(buffSize), _offset(0)
        //{
        //    buffer_type stagingBuffer{};
        //    buffer_memory_type stagingBufferMemory{};
        //    //create and copy data to the staging buffer
        //    cyContext.getAllocator()->createBuffer(BufferCreationAllocationInfo::createDefaultStagingBufferInfo(bufferSize()), stagingBuffer, stagingBufferMemory, data);

        //    //create the index buffer and map its memory                                                                        //ensure that  transfer dst bit is set.
        //    cyContext.getAllocator()->createBuffer(BufferCreationAllocationInfo::createGPUOnlyBufferInfo(bufferSize(), usage), _buffer, _bufferMemory);

        //    //transfer the data from the staging buffer to the vertex buffer.
        //    cyContext.getAllocator()->copyBuffer(stagingBuffer, _buffer, bufferSize());

        //    //cleanup the staging the buffer.
        //    cyContext.getAllocator()->destroyBuffer(stagingBuffer, stagingBufferMemory);

        //    _mapped = true;
        //}

        template<typename V, typename I>
        VulkanBuffer(VulkanContext& context, buffer_size_type vBuffSize, V* vData, buffer_size_type iBuffSize, I* iData)
            :
            cyContext(context), _count(iBuffSize / sizeof(I)), _instanceCount(1), _bufferSize(vBuffSize + iBuffSize), _offset(vBuffSize)
        {
            buffer_type stagingBuffer{};
            buffer_memory_type stagingBufferMemory{};
            buffer_info_type buffInfo = buffer_info_type::createDefaultStagingBufferInfo(_bufferSize);
            //create and copy data to the staging buffer
            cyContext.getAllocator()->createBuffer(buffInfo, stagingBuffer, stagingBufferMemory);

            std::vector<OffsetsInfo> offsetInfo = {
                {vData, vBuffSize, 0},
                {iData, iBuffSize, vBuffSize}
            };
            cyContext.getAllocator()->fillBuffer(buffInfo.allocInfo, stagingBufferMemory, _bufferSize, offsetInfo);

            buffer_info_type thisBuffersInfo = buffer_info_type::createDefaultVertexIndexSharedBufferInfo(_bufferSize);
            //create the index buffer and map its memory
            cyContext.getAllocator()->createBuffer(thisBuffersInfo, _buffer, _bufferMemory);

            //transfer the data from the staging buffer to the vertex buffer.
            cyContext.getAllocator()->copyBuffer(stagingBuffer, _buffer, _bufferSize);

            //cleanup the staging the buffer.
            cyContext.getAllocator()->destroyBuffer(stagingBuffer, stagingBufferMemory);

            //store the buffers setup for later.
            _bufferInfo = thisBuffersInfo;

            _mapped = true;
        }

        ~VulkanBuffer();

        //cannot be created without a context.
        VulkanBuffer() = delete;
        //cannot be copied or reassigned.
        VulkanBuffer(const VulkanBuffer&) = delete;
        VulkanBuffer(VulkanBuffer&&) = delete;
        VulkanBuffer& operator=(const VulkanBuffer&) = delete;

        template<typename T>
        void setData(T* data, offset_type offset)
        {
            ASSERT_ERROR(DEFAULT_LOGGABLE, _buffer != nullptr && _bufferMemory != nullptr, "Buffer has not been initialized.");
            cyContext.getAllocator()->fillBuffer(_bufferInfo.allocInfo, _bufferMemory, _bufferInfo.bufferInfo.size, { {data, _bufferInfo.bufferInfo.size, 0} });
        }

        void setData(void* data, buffer_info_type bufferInfo);

        void create(buffer_info_type bufferInfo);
        void copyTo(VulkanBuffer* destination, VkDeviceSize size);

        element_count_type count() { return _count; }
        instance_count_type instanceCount() { return _instanceCount; }
        buffer_size_type bufferSize() { return _bufferInfo.bufferInfo.size; }
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


