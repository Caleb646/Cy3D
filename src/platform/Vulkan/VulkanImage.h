#pragma once
#include "VulkanAllocator.h"
#include "VulkanContext.h"

namespace cy3d
{
	class VulkanImage
	{
	public:
		using image_type = VulkanAllocator::image_type;
		using image_memory_type = VulkanAllocator::image_memory_type;
		using image_info_type = VulkanAllocator::image_info_type;
		using image_view_type = VkImageView;

		using buffer_type = VulkanAllocator::buffer_type;
		using buffer_memory_type = VulkanAllocator::buffer_memory_type;
		using buffer_info_type = VulkanAllocator::buffer_info_type;
		
		

	private:
		image_type _image{ nullptr };
		image_memory_type _imageMemory{ nullptr };
		image_view_type _imageView{ nullptr };
		image_info_type _imageInfo;

		VulkanContext& cyContext;

	public:
		VulkanImage(VulkanContext&, image_info_type, void*);
		VulkanImage(VulkanContext& context, image_info_type imageInfo);
		~VulkanImage();
		void cleanup();

		void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		image_type& getImage() { return _image; }
		VkFormat getFormat() { return _imageInfo.imageInfo.format; }
		VkDeviceSize getImageSize() { return _imageInfo.imageInfo.imageSize; }
		VkImageAspectFlags getAspectFlags() { return _imageInfo.imageInfo.aspectFlags; }
		VkImageView& getImageView() { return _imageView; }

		static std::unique_ptr<VulkanImage> createDepthImage(VulkanContext& context, uint32_t width, uint32_t height);

	private:
		void init();
	};
}


