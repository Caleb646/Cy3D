#include "pch.h"

#include <stb_image/stb_image.h>

#include "VulkanTexture.h"
#include "VulkanDevice.h"


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
        ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateSampler(cyContext.getDevice()->device(), &_samplerInfo.samplerInfo, nullptr, &_sampler) == VK_SUCCESS, "Failed to create sampler");
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
        ASSERT_ERROR(DEFAULT_LOGGABLE, pixels != nullptr, "Texture failed to load.");

        //create info needed for image buffer creation
        VulkanImageBuffer::image_info_type imageInfo = VulkanImageBuffer::image_info_type::createDefaultImage(texWidth, texHeight, imageSize);
        _texture.reset(new VulkanImageBuffer(context, imageInfo, pixels));

        //cleanup pixel array.
        stbi_image_free(pixels);
        //create image view to access the image
        _textureImageView = cyContext.getDevice()->createImageView(_texture->getImage(), VK_FORMAT_R8G8B8A8_SRGB);
        //create texture sampler
        _sampler.reset(new VulkanSampler(cyContext));

	}

    VulkanTexture::~VulkanTexture()
    {
        cleanup();
    }

    void VulkanTexture::cleanup()
    {
        if (_texture != nullptr)
        {
            vkDestroyImageView(cyContext.getDevice()->device(), _textureImageView, nullptr);
        }
    }
}
