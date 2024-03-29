#pragma once
#include "pch.h"

#include <Logi/Logi.h>

#include "Vulkan.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanAllocator.h"


namespace cy3d
{

    struct Vertex {
        m3d::vec3f pos;
        m3d::vec3f color;
        m3d::vec3f texCoord;

        /**
         * @brief A vertex binding describes at which rate to load data from memory throughout the vertices.
         * It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance.
         * @return
        */
        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);

            /**
             * VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
             * VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance: for instance rendering
            */
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return bindingDescription;
        }

        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
        {
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions{ 3 };
            attributeDescriptions[0].binding = 0; //The binding parameter tells Vulkan from which binding the per-vertex data comes. 
            attributeDescriptions[0].location = 0; //The location parameter references the location directive of the input in the vertex shader.

            /**
             * float: VK_FORMAT_R32_SFLOAT
             * vec2: VK_FORMAT_R32G32_SFLOAT
             * vec3: VK_FORMAT_R32G32B32_SFLOAT
             * vec4: VK_FORMAT_R32G32B32A32_SFLOAT
            */
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; //The format parameter describes the type of data for the attribute.
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            return attributeDescriptions;
        }
    };

	class VulkanBuffer
	{

    public:
        using buffer_type = typename VulkanAllocator::buffer_type;
        using buffer_memory_type = typename VulkanAllocator::buffer_memory_type;
        using offsets_type = typename VulkanAllocator::offsets_type;

    private:
        buffer_type _buffer{ nullptr };
        buffer_memory_type _bufferMemory{ nullptr };
        BufferCreateInfo _bufferInfo;
        VulkanContext& cyContext;

        uint32_t _count;
        uint32_t _instanceCount;
        VkDeviceSize _bufferSize;
        VkDeviceSize _offset;

        //tracks if the buffer and buffer memory have been mapped
        bool _mapped{ false };

    public:
        VulkanBuffer(VulkanContext& context, BufferCreateInfo bufferInfo, uint32_t count = 1)
            :
            cyContext(context), _bufferInfo(bufferInfo), _count(count), _instanceCount(1), _bufferSize(bufferInfo.bufferInfo.size), _offset(0)
        {
            cyContext.getAllocator()->createBuffer(_bufferInfo, _buffer, _bufferMemory);
        }

        template<typename T>
        VulkanBuffer(VulkanContext& context, BufferCreateInfo bufferInfo, T* data)
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
        //    cyContext.getAllocator()->createBuffer(BufferCreateInfo::createDefaultStagingBufferInfo(bufferSize()), stagingBuffer, stagingBufferMemory, data);

        //    //create the index buffer and map its memory                                                                        //ensure that  transfer dst bit is set.
        //    cyContext.getAllocator()->createBuffer(BufferCreateInfo::createGPUOnlyBufferInfo(bufferSize(), usage), _buffer, _bufferMemory);

        //    //transfer the data from the staging buffer to the vertex buffer.
        //    cyContext.getAllocator()->copyBuffer(stagingBuffer, _buffer, bufferSize());

        //    //cleanup the staging the buffer.
        //    cyContext.getAllocator()->destroyBuffer(stagingBuffer, stagingBufferMemory);

        //    _mapped = true;
        //}

        template<typename V, typename I>
        VulkanBuffer(VulkanContext& context, VkDeviceSize vBuffSize, V* vData, VkDeviceSize iBuffSize, I* iData)
            :
            cyContext(context), _count(iBuffSize / sizeof(I)), _instanceCount(1), _bufferSize(vBuffSize + iBuffSize), _offset(vBuffSize)
        {
            buffer_type stagingBuffer{};
            buffer_memory_type stagingBufferMemory{};
            BufferCreateInfo buffInfo = BufferCreateInfo::createDefaultStagingBufferInfo(_bufferSize);
            //create and copy data to the staging buffer
            cyContext.getAllocator()->createBuffer(buffInfo, stagingBuffer, stagingBufferMemory);

            std::vector<OffsetsInfo> offsetInfo = {
                {vData, vBuffSize, 0},
                {iData, iBuffSize, vBuffSize}
            };
            cyContext.getAllocator()->fillBuffer(buffInfo.allocInfo, stagingBufferMemory, _bufferSize, offsetInfo);

            BufferCreateInfo thisBuffersInfo = BufferCreateInfo::createDefaultVertexIndexSharedBufferInfo(_bufferSize);
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
        void setData(T* data, uint32_t offset = 0)
        {
            CY_ASSERT(_buffer != nullptr && _bufferMemory != nullptr);
            cyContext.getAllocator()->fillBuffer(_bufferInfo.allocInfo, _bufferMemory, _bufferInfo.bufferInfo.size, { {data, _bufferInfo.bufferInfo.size, offset} });
        }

        VkDescriptorBufferInfo descriptorInfo(VkDeviceSize buffSize = VK_WHOLE_SIZE, uint32_t offset = 0)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = getBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = buffSize;
            return bufferInfo;
        }

        uint32_t count() { return _count; }
        uint32_t instanceCount() { return _instanceCount; }
        VkDeviceSize bufferSize() { return _bufferInfo.bufferInfo.size; }
        uint32_t offset() { return _offset; }

        buffer_type getBuffer()
        {
            CY_ASSERT(_buffer != nullptr);
            return _buffer;
        }

    private:
        void cleanup();
	};
}


