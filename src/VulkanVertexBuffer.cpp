#include "pch.h"

#include "VulkanVertexBuffer.h"

#include <Logi/Logi.h>


namespace cy3d
{
	VulkanVertexBuffer::VulkanVertexBuffer(std::vector<Vertex>& verts)
	{
		init(verts);
	}
	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		//vkDestroyBuffer(device, vertexBuffer, nullptr);
	}

	void VulkanVertexBuffer::init(std::vector<Vertex>& verts)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(verts[0]) * verts.size();
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		//if (vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
			//throw std::runtime_error("failed to create vertex buffer!");
		//}
	}
}
