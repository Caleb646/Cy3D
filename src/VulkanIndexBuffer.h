#pragma once
#include "pch.h"

//#include <Logi/Logi.h>
//
//#include "Fwd.hpp"
//
//namespace cy3d
//{
//	class VulkanIndexBuffer
//	{
//    private:
//        VkBuffer indexBuffer{ nullptr };
//        VkDeviceMemory indexBufferMemory{ nullptr };
//        VulkanContext& cyContext;
//        VkDeviceSize _bufferSize;
//        //tracks if the buffer and buffer memory have been mapped
//        bool mapped{ false };
//
//    public:
//        //TODO add size, void * data, usage to constructor
//        VulkanIndexBuffer(VulkanContext&, VkDeviceSize s, void* data);
//        ~VulkanIndexBuffer();
//
//        //cannot be created without a context.
//        VulkanIndexBuffer() = delete;
//        //cannot be copied or reassigned.
//        VulkanIndexBuffer(const VulkanIndexBuffer&) = delete;
//        VulkanIndexBuffer(VulkanIndexBuffer&&) = delete;
//        VulkanIndexBuffer& operator=(const VulkanIndexBuffer&) = delete;
//
//
//        void setData(void* data, VkDeviceSize size);
//        void setData(void* data, BufferCreationAllocationInfo bufferInfo);
//
//        void create(BufferCreationAllocationInfo bufferInfo);
//        void copyTo(VulkanIndexBuffer* destination, VkDeviceSize size);
//
//        VkDeviceSize bufferSize() { return _bufferSize; }
//
//        VkBuffer getIndexBuffer()
//        {
//            ASSERT_ERROR(DEFAULT_LOGGABLE, indexBuffer != nullptr, "Data has not been set. Buffer is null.");
//            return indexBuffer;
//        }
//
//    private:
//        void cleanup();
//	};
//}


