#pragma once
#include "VulkanAllocator.h"
#include "VulkanContext.h"

namespace cy3d
{
	class VulkanImageBuffer
	{
	public:
		using image_type = VulkanAllocator::image_type;
		using image_memory_type = VulkanAllocator::image_memory_type;
		using image_info_type = VulkanAllocator::image_info_type;
		using buffer_type = VulkanAllocator::buffer_type;
		using buffer_memory_type = VulkanAllocator::buffer_memory_type;
		using buffer_info_type = VulkanAllocator::buffer_info_type;

	private:
		image_type _image{ nullptr };
		image_memory_type _imageMemory{ nullptr };
		image_info_type _imageInfo;

		VulkanContext& cyContext;

	public:
		VulkanImageBuffer(VulkanContext&, image_info_type, void*);
		~VulkanImageBuffer();

		void cleanup();

		void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		image_type& getImage() { return _image; }
	};
}


