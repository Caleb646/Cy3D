#pragma once
#include "pch.h"

#include <Logi/Logi.h>

#include "Vulkan.h"
#include "VulkanContext.h"
#include "VulkanAllocator.h"
#include "VulkanImage.h"
#include "VulkanDevice.h"

namespace cy3d
{
	struct SamplerCreationInfo
	{
		VkSamplerCreateInfo samplerInfo{};
		VkPhysicalDeviceProperties properties{};

		static SamplerCreationInfo createDefaultSampler(VulkanContext& context)
		{
			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(context.getDevice()->physicalDevice(), &properties);

			samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 0.0f;

			return SamplerCreationInfo{ samplerInfo, properties };
		}
	};

	class VulkanSampler
	{
	public:
		using sampler_type = VkSampler;
		using sampler_info_type = SamplerCreationInfo;
	private:
		sampler_type _sampler{ nullptr };
		sampler_info_type _samplerInfo;
		VulkanContext& cyContext;

	public:
		VulkanSampler(VulkanContext& context);
		~VulkanSampler();
		void cleanup();

		sampler_type& getSampler() 
		{
			CY_ASSERT(_sampler != nullptr);
			return _sampler;
		}
	};



	class VulkanTexture
	{
	public:
		using image_view_type = VkImageView;
	private:
		std::string _path;
		std::unique_ptr<VulkanImage> _texture{ nullptr };
		std::unique_ptr<VulkanSampler> _sampler;
		VulkanContext& cyContext;

	public:
		VulkanTexture(VulkanContext& context, std::string path);
		~VulkanTexture();
		void cleanup();

		VkDescriptorImageInfo descriptorInfo()
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = _texture->getImageView();
			imageInfo.sampler = _sampler->getSampler();
			return imageInfo;
		}
	};
}


