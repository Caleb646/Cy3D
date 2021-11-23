#pragma once
#include "pch.h"

//#include "Fwd.hpp"
#include <M3D/M3D.h>
namespace cy3d
{
    //
    //
    struct Vertex {
        m3d::Vec2f pos;
        m3d::Vec3f color;

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
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions{ 2 };
            attributeDescriptions[0].binding = 0; //The binding parameter tells Vulkan from which binding the per-vertex data comes. 
            attributeDescriptions[0].location = 0; //The location parameter references the location directive of the input in the vertex shader.

            /**
             * float: VK_FORMAT_R32_SFLOAT
             * vec2: VK_FORMAT_R32G32_SFLOAT
             * vec3: VK_FORMAT_R32G32B32_SFLOAT
             * vec4: VK_FORMAT_R32G32B32A32_SFLOAT
            */
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; //The format parameter describes the type of data for the attribute.
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);


            return attributeDescriptions;
        }
    };
}
//
//	/**
//	 * @brief 
//     * @note as long as the object this class belongs to doesnt outlive the Device class the 
//     * default destructor will work.
//	*/
//	class VulkanVertexBuffer
//	{
//    private:
//        VkBuffer vertexBuffer{ nullptr };
//        VkDeviceMemory vertexBufferMemory{ nullptr };
//        VulkanContext& cyContext;
//        VkDeviceSize _bufferSize;
//        //tracks if the buffer and buffer memory have been mapped
//        bool mapped{ false };
//
//    public:
//        //TODO add size, void * data, usage to constructor
//        VulkanVertexBuffer(VulkanContext&, VkDeviceSize s, void* data);
//        ~VulkanVertexBuffer();
//
//        //cannot be created without a context.
//        VulkanVertexBuffer() = delete;
//        //cannot be copied or reassigned.
//        VulkanVertexBuffer(const VulkanVertexBuffer&) = delete;
//        VulkanVertexBuffer(VulkanVertexBuffer&&) = delete;
//        VulkanVertexBuffer& operator=(const VulkanVertexBuffer&) = delete;
//
//
//        void setData(void* data, VkDeviceSize size);
//        void setData(void* data, BufferCreationAllocationInfo bufferInfo);
//
//        void create(BufferCreationAllocationInfo bufferInfo);
//        void copyTo(VulkanVertexBuffer* destination, VkDeviceSize size);
//
//        VkDeviceSize bufferSize() { return _bufferSize; }
//
//        VkBuffer getVertexBuffer() 
//        { 
//            ASSERT_ERROR(DEFAULT_LOGGABLE, vertexBuffer != nullptr, "Data has not been set. Buffer is null.");
//            return vertexBuffer;
//        }
//
//    private:
//        void cleanup();
//	};


