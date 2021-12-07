#include "pch.h"
#include "VulkanImage.h"
#include "VulkanDevice.h"


namespace cy3d
{
	VulkanImage::VulkanImage(VulkanContext& context, image_info_type imageInfo, void* data) : cyContext(context), _imageInfo(imageInfo) 
	{
		init();

		//create and fill staging buffer
		buffer_type stagingBuffer;
		buffer_memory_type stagingMemory;
		buffer_info_type stagingBuffInfo = buffer_info_type::createDefaultStagingBufferInfo(getImageSize());
		cyContext.getAllocator()->createBuffer(stagingBuffInfo, stagingBuffer, stagingMemory);
		cyContext.getAllocator()->fillBuffer(stagingBuffInfo.allocInfo, stagingMemory, stagingBuffInfo.bufferInfo.size, { {data, getImageSize(), 0} });

		//transition image to proper layer before staging buffer is copied into it.
		transitionImageLayout(getFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		//copy staging buffer to image
		cyContext.getAllocator()->copyBufferToImage(stagingBuffer, _image, _imageInfo);

		//transition image from destination to a shader read
		transitionImageLayout(getFormat(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		//cleanup staging buffer
		cyContext.getAllocator()->destroyBuffer(stagingBuffer, stagingMemory);

	}

	VulkanImage::VulkanImage(VulkanContext& context, image_info_type imageInfo) : cyContext(context), _imageInfo(imageInfo)
	{
		init();
	}

	VulkanImage::~VulkanImage()
	{
		cleanup();
	}

	void VulkanImage::cleanup()
	{
		if (_image != nullptr && _imageMemory != nullptr)
		{
			cyContext.getAllocator()->destroyImage(_image, _imageMemory);
		}

		if (_imageView != nullptr)
		{
			vkDestroyImageView(cyContext.getDevice()->device(), _imageView, nullptr);
		}
	}

	void VulkanImage::init()
	{
		cyContext.getAllocator()->createImage(_imageInfo, _image, _imageMemory);
		_imageView = cyContext.getDevice()->createImageView(_image, getFormat(), getAspectFlags());
	}

	void VulkanImage::transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = cyContext.getDevice()->beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = _image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}

		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		else
		{
			CY_ASSERT(false);
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		cyContext.getDevice()->endSingleTimeCommands(commandBuffer);
	}

	/**
	* 
	* 
	* 
	* Public Static Methods
	* 
	* 
	* 
	*/
	std::unique_ptr<VulkanImage> VulkanImage::createDepthImage(VulkanContext& context, uint32_t width, uint32_t height)
	{
		ImageInfo info
		{ 
			context.getDevice()->findDepthFormat(), 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			width,
			height,
			width*height
		};

		ImageCreationAllocationInfo imageInfo = ImageCreationAllocationInfo::createDefaultImageInfo(info);

		return std::make_unique<VulkanImage>(context, imageInfo);
	}
}
