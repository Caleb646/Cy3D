#pragma once

#include "platform/Vulkan/VulkanTexture.h"

#include "define.h"

namespace cy3d
{
	//struct Vertex
	//{
	//	m3d::vec3f position;
	//	m3d::vec3f normal;
	//	m3d::vec2f texCoords;
	//	m3d::vec3f tangent;
	//	m3d::vec3f bitTangent;

 //       static VkVertexInputBindingDescription getBindingDescription()
 //       {
 //           VkVertexInputBindingDescription bindingDescription{};
 //           bindingDescription.binding = 0;
 //           bindingDescription.stride = sizeof(Vertex);

 //           /**
 //            * VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
 //            * VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance: for instance rendering
 //           */
 //           bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
 //           return bindingDescription;
 //       }

 //       static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
 //       {
 //           std::vector<VkVertexInputAttributeDescription> attributeDescriptions{ 5 };
 //           attributeDescriptions[0].binding = 0; //The binding parameter tells Vulkan from which binding the per-vertex data comes. 
 //           attributeDescriptions[0].location = 0; //The location parameter references the location directive of the input in the vertex shader.

 //           /**
 //            * float: VK_FORMAT_R32_SFLOAT
 //            * vec2: VK_FORMAT_R32G32_SFLOAT
 //            * vec3: VK_FORMAT_R32G32B32_SFLOAT
 //            * vec4: VK_FORMAT_R32G32B32A32_SFLOAT
 //           */
 //           attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; //The format parameter describes the type of data for the attribute.
 //           attributeDescriptions[0].offset = offsetof(Vertex, position);

 //           attributeDescriptions[1].binding = 0;
 //           attributeDescriptions[1].location = 1;
 //           attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
 //           attributeDescriptions[1].offset = offsetof(Vertex, normal);

 //           attributeDescriptions[2].binding = 0;
 //           attributeDescriptions[2].location = 2;
 //           attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
 //           attributeDescriptions[2].offset = offsetof(Vertex, texCoords);

 //           attributeDescriptions[3].binding = 0;
 //           attributeDescriptions[3].location = 3;
 //           attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
 //           attributeDescriptions[3].offset = offsetof(Vertex, tangent);

 //           attributeDescriptions[4].binding = 0;
 //           attributeDescriptions[4].location = 4;
 //           attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
 //           attributeDescriptions[4].offset = offsetof(Vertex, bitTangent);

 //           return attributeDescriptions;
 //       }
	//};

	//struct Texture 
	//{
	//	std::string type;
	//	std::string path;
 //       std::unique_ptr<VulkanTexture> textureBuffer{ nullptr };

 //       CY_NOCOPY(Texture);
	//};

	//struct Mesh
	//{
	//	std::vector<Vertex> verts;
	//	//std::vector<Texture> textures;
	//	std::vector<uint32_t> indices;
	//};
}

