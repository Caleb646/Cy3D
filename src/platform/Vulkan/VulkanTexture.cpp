#include "pch.h"

#include <stb_image/stb_image.h>

#include "VulkanTexture.h"


namespace cy3d
{
    /**
    * 
    * 
    * 
    * Vulkan Sampler
    * 
    * 
    * 
    */
    VulkanSampler::VulkanSampler(VulkanContext& context) : cyContext(context)
    {
        _samplerInfo = sampler_info_type::createDefaultSampler(context);
        VK_CHECK(vkCreateSampler(cyContext.getDevice()->device(), &_samplerInfo.samplerInfo, nullptr, &_sampler));
    }

    VulkanSampler::~VulkanSampler()
    {
        cleanup();
    }

    void VulkanSampler::cleanup()
    {
        if (_sampler != nullptr)
        {
            vkDestroySampler(cyContext.getDevice()->device(), _sampler, nullptr);
        }   
    }

    /**
    *
    *
    *
    * Vulkan Texture
    *
    *
    *
    */
	VulkanTexture::VulkanTexture(VulkanContext& context, std::string path) : cyContext(context), _path(path)
	{
        int texWidth, texHeight, texChannels;
        //load texture
        stbi_uc* pixels = stbi_load(_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        /**
         * The pixels are laid out row by row with 4 bytes per pixel in the case of STBI_rgb_alpha for a total of texWidth * texHeight * 4 values. 
        */
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        CY_ASSERT(pixels != nullptr);

        //create info needed for image buffer creation
        ImageInfo baseInfo{ VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, texWidth, texHeight, imageSize };
        VulkanImage::image_info_type imageInfo = VulkanImage::image_info_type::createDefaultImageInfo(baseInfo);
        _texture.reset(new VulkanImage(context, imageInfo, pixels));

        //cleanup pixel array.
        stbi_image_free(pixels);

        //create texture sampler
        _sampler.reset(new VulkanSampler(cyContext));

	}

    VulkanTexture::~VulkanTexture()
    {
        cleanup();
    }

    void VulkanTexture::cleanup()
    {

    }
}
