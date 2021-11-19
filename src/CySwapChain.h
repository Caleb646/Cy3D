#pragma once

#include "pch.h"

#include "CyDevice.h"
#include "CyPipeline.h"


namespace cy3d {


    /**
     * @brief There are three types of settings to determine: Surface format (color depth), Presentation mode (conditions for "swapping" images to the screen)
     * and Swap extent (resolution of images in swap chain).
    */

    class CySwapChain {

    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    private:
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass;

        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        CyDevice& cyDevice;
        CyWindow& cyWindow;
        std::unique_ptr<CyPipeline> cyPipeline;
        VkPipelineLayout pipelineLayout;

        std::vector<VkCommandBuffer> commandBuffers;

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

        //To use the right pair of semaphores every time, we need to keep track of the current frame
        std::size_t currentFrame = 0;

    public:

        CySwapChain(CyDevice& d, CyWindow& w);
        ~CySwapChain();

        CySwapChain(const CySwapChain&) = delete;
        void operator=(const CySwapChain&) = delete;

        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
        VkRenderPass getRenderPass() { return renderPass; }
        VkImageView getImageView(int index) { return swapChainImageViews[index]; }

        /**
         * @brief 
         * @return the current number of Swap Chain Images.
        */
        std::size_t imageCount() { return swapChainImages.size(); }
        VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
        VkExtent2D getSwapChainExtent() { return swapChainExtent; }
        uint32_t width() { return swapChainExtent.width; }
        uint32_t height() { return swapChainExtent.height; }

        float extentAspectRatio() { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }
        VkFormat findDepthFormat();
        void resetFences(std::size_t frameNumber);
        VkResult acquireNextImage(uint32_t* imageIndex);
        VkResult submitCommandBuffers(uint32_t* imageIndex);

    private:

        void cleanup();
        void recreate();

        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();
        void createDefaultPipelineLayout();
        void createDefaultPipeline();
        void createCommandBuffers();

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    };

}

