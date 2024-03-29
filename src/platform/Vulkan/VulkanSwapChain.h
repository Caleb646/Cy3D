#pragma once
#include "pch.h"

#include "VulkanDevice.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanDescriptors.h"
#include "VulkanTexture.h"
#include "Fwd.hpp"


namespace cy3d {


    struct UniformBufferObject
    {
        alignas(16) m3d::mat4f model;
        alignas(16) m3d::mat4f view;
        alignas(16) m3d::mat4f proj;

        void update(float width, float height)
        {
            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            
            /*model.getRotatedZ(time * m3d::degreesToRadians(90.0f));
            view = m3d::mat4f::getLookAt({ 2.0f, 2.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f, 1.0f});
            proj = m3d::mat4f::getPerspective(m3d::degreesToRadians(45.0f), width / height, 0.1f, 10.0f);
            proj(1, 1) *= -1;*/
        }
    };
    //TODO temporary



    /**
     * @brief The swap chain is essentially a queue of images that are waiting to be presented to the screen. 
     * Our application will acquire such an image to draw to it, and then return it to the queue. 
     
     * There are three types of settings to determine for the swap chain: Surface format (color depth), 
     * Presentation mode (conditions for "swapping" images to the screen)
     * and Swap extent (resolution of images in swap chain).
    */
    class VulkanSwapChain {

    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    private:
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        /**
         * Before we can finish creating the pipeline, we need to tell Vulkan about the framebuffer attachments
         * that will be used while rendering. We need to specify how many color and depth buffers there will be,
         * how many samples to use for each of them and how their contents should be handled throughout the rendering
         * operations. All of this information is wrapped in a render pass object.
        */
        VkRenderPass _renderPass;

        /**
         * A framebuffer object references all of the VkImageView objects that represent the attachments. 
         * In our case that will be only a single one: the color attachment. However, the image that we have 
         * to use for the attachment depends on which image the swap chain returns when we retrieve one for presentation. 
         * That means that we have to create a framebuffer for all of the images in the swap chain and use the one that 
         * corresponds to the retrieved image at drawing time.
        */
        std::vector<VkFramebuffer> swapChainFramebuffers;

        std::vector<std::unique_ptr<VulkanImage>> depthImages;

        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        VulkanContext& cyContext;

        /**
         * Vulkan works with pixels, so the swap chain extent must be specified in pixels as well. 
         * if you are using a high DPI display (like Apple's Retina display), screen coordinates don't correspond to pixels.
         * Instead, due to the higher pixel density, the resolution of the window in pixel will be larger than the resolution in 
         * screen coordinates. So if Vulkan doesn't fix the swap extent for us, we can't just use the original {WIDTH, HEIGHT}. 
         * Instead, we must use glfwGetFramebufferSize to query the resolution of the window in pixel before matching it against 
         * the minimum and maximum image extent.
        */
        //VkExtent2D windowExtent;

        VkSwapchainKHR swapChain;

        std::vector<VkSemaphore> imageAvailableSemaphores; //signals that an image has been acquired and is ready for rendering
        std::vector<VkSemaphore> renderFinishedSemaphores; //signals that rendering has finished and presentation can happen

        /**
         * To perform CPU-GPU synchronization, Vulkan offers a second type of 
         * synchronization primitive called fences. Fences are similar to 
         * semaphores in the sense that they can be signaled and waited for, but this time we actually wait for them in our own code.
        */
        std::vector<VkFence> inFlightFences;
        /**
         * If MAX_FRAMES_IN_FLIGHT is higher than the number of swap chain images or vkAcquireNextImageKHR returns images out-of-order 
         * then it's possible that we may start rendering to a swap chain image that is already in flight. To avoid this, we need to track 
         * for each swap chain image if a frame in flight is currently using it. 
        */
        std::vector<VkFence> imagesInFlight;

    public:

        //VulkanSwapChain(VulkanDevice& d, VulkanWindow& w);
        VulkanSwapChain(VulkanContext& context);
        ~VulkanSwapChain();

        void reCreate();

        VulkanSwapChain() = delete;
        VulkanSwapChain(VulkanSwapChain&&) = delete;
        VulkanSwapChain(const VulkanSwapChain&) = delete;
        void operator=(const VulkanSwapChain&) = delete;

        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
        VkImageView getImageView(int index) { return swapChainImageViews[index]; }

        /**
         * @brief 
         * @return the current number of Swap Chain Images.
        */
        std::size_t imageCount() { return swapChainImages.size(); }
        VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
        VkExtent2D getSwapChainExtent() { return swapChainExtent; }
        uint32_t getWidth() { return swapChainExtent.width; }
        uint32_t getHeight() { return swapChainExtent.height; }
        VkRenderPass& getRenderPass() { return _renderPass; }

        float extentAspectRatio() { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }
        void resetFences(std::size_t frameNumber);
        VkResult acquireNextImage(uint32_t* imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

    private:
        void cleanup();  

        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    };

}

