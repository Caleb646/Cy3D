#include "pch.h"

#include "VulkanSwapChain.h"
#include "VulkanContext.h"

#include <Logi/Logi.h>


namespace cy3d {

    VulkanSwapChain::VulkanSwapChain(VulkanContext& context) : cyContext(context)
    {
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDepthResources();
        createIndexBuffers();
        createVertexBuffers();
        createFramebuffers();
        createSyncObjects();

        createDefaultPipelineLayout();
        createDefaultPipeline();
        createCommandBuffers();
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        cleanup();

        // cleanup synchronization objects
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(cyContext.getDevice()->device(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(cyContext.getDevice()->device(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(cyContext.getDevice()->device(), inFlightFences[i], nullptr);
        }
    }

    /**
     * @brief Destroys and Frees the depth images/views/memories, framebuffers, command buffers (NOT command pool),
     * pipeline layout, pipeline, renderpass, swapchain image view, and the VkSwapChainKHR.
    */
    void VulkanSwapChain::cleanup()
    {
        for (int i = 0; i < depthImages.size(); i++)
        {
            vkDestroyImageView(cyContext.getDevice()->device(), depthImageViews[i], nullptr);
            vkDestroyImage(cyContext.getDevice()->device(), depthImages[i], nullptr);
            vkFreeMemory(cyContext.getDevice()->device(), depthImageMemorys[i], nullptr);
        }

        for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
        {
            vkDestroyFramebuffer(cyContext.getDevice()->device(), swapChainFramebuffers[i], nullptr);
        }
        vkFreeCommandBuffers(cyContext.getDevice()->device(), cyContext.getDevice()->getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

        //let std::unique_ptr cleanup the cyPipeline.
        
        vkDestroyPipelineLayout(cyContext.getDevice()->device(), pipelineLayout, nullptr);
        vkDestroyRenderPass(cyContext.getDevice()->device(), renderPass, nullptr);

        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            vkDestroyImageView(cyContext.getDevice()->device(), swapChainImageViews[i], nullptr);
        }

        if (swapChain != nullptr)
        {
            vkDestroySwapchainKHR(cyContext.getDevice()->device(), swapChain, nullptr);
            swapChain = nullptr;
        }
    }

    /**
     * @brief Recreates the swap chain. Will first block until the window is not minimized, then will block until the device is idle
     * and, then begin cleaning up and recreating the swap chain with the new window extent.
    */
    void VulkanSwapChain::recreate()
    {
        //if window is currently minimized block
        cyContext.getWindow()->blockWhileWindowMinimized();

        //make sure nothing is currently being used 
        //before cleaning up.
        vkDeviceWaitIdle(cyContext.getDevice()->device());

        cleanup();

        createSwapChain();
        createImageViews();
        createRenderPass();
        createDepthResources();
        createFramebuffers();

        createDefaultPipelineLayout();
        createDefaultPipeline();
        createCommandBuffers();

        //because sync objects are being reused the imagesInFlight need to be reset here.
        imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);
    }

    VkResult VulkanSwapChain::acquireNextImage(uint32_t* imageIndex)
    {
        /**
         * The vkWaitForFences function takes an array of fences and waits for either any or all of them to be signaled before returning. 
         * The VK_TRUE we pass here indicates that we want to wait for all fences
        */
        vkWaitForFences(cyContext.getDevice()->device(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        /**
         * Acquire an image from the swap chain.
         * The first two parameters of vkAcquireNextImageKHR are the logical device and the swap chain from which we 
         * wish to acquire an image. The third parameter specifies a timeout in nanoseconds for an image to become available. 
         * Using the maximum value of a 64 bit unsigned integer disables the timeout.
         * 
         * The next two parameters specify synchronization objects that are to be signaled when the presentation engine is 
         * finished using the image. That's the point in time where we can start drawing to it. It is possible to specify a semaphore, fence or both.
         * We're going to use our imageAvailableSemaphore for that purpose.
         * 
         * The last parameter specifies a variable to output the index of the swap chain image that has become available. 
         * The index refers to the VkImage in our swapChainImages array.
        */
        VkResult result = vkAcquireNextImageKHR(cyContext.getDevice()->device(), swapChain, UINT64_MAX,
            imageAvailableSemaphores[currentFrame],  //must be a not signaled semaphore
            VK_NULL_HANDLE, imageIndex);

        /**
         * If the swap chain turns out to be out of date when attempting to acquire an image, 
         * then it is no longer possible to present to it. Therefore we should immediately recreate 
         * the swap chain and try again in the next drawFrame call. 
        */
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreate();
            return result;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        return result;
    }

    void VulkanSwapChain::resetFences(std::size_t frameNumber)
    {
        vkResetFences(cyContext.getDevice()->device(), 1, &inFlightFences[currentFrame]);
    }

    VkResult VulkanSwapChain::submitCommandBuffers(uint32_t* imageIndex)
    {
        //Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
        {
            vkWaitForFences(cyContext.getDevice()->device(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
        }
        //Mark the image as now being in use by this frame
        imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        //These three parameters specify which semaphores to wait on before execution begins and in which stage(s) of the pipeline to wait. 
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        //These two parameters specify which command buffers to actually submit for execution
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[*imageIndex];

        //The signalSemaphoreCount and pSignalSemaphores parameters specify which semaphores to signal once the command buffer(s) have finished execution
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        resetFences(currentFrame);

        /**
         * The function takes an array of VkSubmitInfo structures as argument for efficiency when the 
         * workload is much larger. The last parameter references an optional fence that will be signaled when the command buffers finish execution. 
         * 
         * The vkQueueSubmit call includes an optional parameter to pass a fence that should be signaled when the command buffer finishes executing. 
         * We can use this to signal that a frame has finished.
        */
        ASSERT_ERROR(DEFAULT_LOGGABLE, vkQueueSubmit(cyContext.getDevice()->graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) == VK_SUCCESS, "Failed to submit draw command buffer");


        /**
         * The last step of drawing a frame is submitting the result back to 
         * the swap chain to have it eventually show up on the screen. 
        */
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        //specify which semaphores to wait on before presentation can happen
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        //specify the swap chains to present images to and the index of the image for each swap chain
        VkSwapchainKHR swapChains[] = { swapChain };   
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = imageIndex;

        //The vkQueuePresentKHR function submits the request to present an image to the swap chain.
        auto result = vkQueuePresentKHR(cyContext.getDevice()->presentQueue(), &presentInfo);

        /**
         * If the swap chain turns out to be out of date when attempting to acquire an image,
         * then it is no longer possible to present to it. Therefore we should immediately recreate
         * the swap chain and try again in the next drawFrame call.
        */
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || cyContext.getWindow()->isWindowFrameBufferResized()) 
        {
            cyContext.getWindow()->resetWindowFrameBufferResizedFlag();
            recreate();
            return result;
        }
        else if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    void VulkanSwapChain::createSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = cyContext.getDevice()->getSwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        /**
         * Aside from these properties we also have to decide how many images we would 
         * like to have in the swap chain. The implementation specifies the minimum number that it requires to function. 
         * However, simply sticking to this minimum means that we may sometimes have to wait on the driver to complete 
         * internal operations before we can acquire another image to render to. Therefore it is recommended to request at least one more image than the minimum:
        */
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        /**
         * We should also make sure to not exceed the maximum number of images while doing this, where 0 
         * is a special value that means that there is no maximum.
        */
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = cyContext.getDevice()->surface();

        /**
         * The imageArrayLayers specifies the amount of layers each image consists of. This is always 
         * 1 unless you are developing a stereoscopic 3D application. The imageUsage bit field specifies what 
         * kind of operations we'll use the images in the swap chain for.
        */
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        //find setup queues
        QueueFamilyIndices indices = cyContext.getDevice()->findPhysicalQueueFamilies();
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        /**
         * Next, we need to specify how to handle swap chain images that will be used across multiple queue families. 
         * That will be the case in our application if the graphics queue family is different from the presentation queue. We'll be drawing on the 
         * images in the swap chain from the graphics queue and then submitting them on the presentation queue. There are two ways to handle images 
         * that are accessed from multiple queues:

         * VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership must be explicitly transferred before using it in another queue family. This option offers the best performance.
         * 
         * VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit ownership transfers.
        */
        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;      // Optional
            createInfo.pQueueFamilyIndices = nullptr;  // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;


        /**
         * The compositeAlpha field specifies if the alpha channel should be used for blending with other windows in the window system. 
         * You'll almost always want to simply ignore the alpha channel, hence VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
        */
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;

        /**
         * If the clipped member is set to VK_TRUE then that means that we don't care about the color of pixels that are obscured.
        */
        createInfo.clipped = VK_TRUE;

        /**
         * With Vulkan it's possible that your swap chain becomes invalid or unoptimized while your application is running, 
         * for example because the window was resized. In that case the swap chain actually needs to be recreated from scratch and a 
         * reference to the old one must be specified in this field.
        */
        createInfo.oldSwapchain = VK_NULL_HANDLE;


        ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateSwapchainKHR(cyContext.getDevice()->device(), &createInfo, nullptr, &swapChain) == VK_SUCCESS, "Failed to create swap chain");

        /**
         * We only specified a minimum number of images in the swap chain, so the implementation is
         * allowed to create a swap chain with more. That's why we'll first query the final number of
         * images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
         * retrieve the handles.
        */
        vkGetSwapchainImagesKHR(cyContext.getDevice()->device(), swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(cyContext.getDevice()->device(), swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    /**
     * An image view is quite literally a view into an image. It describes how to access the 
     * image and which part of the image to access, for example if it should be treated as a 2D 
     * texture depth texture without any mipmapping levels.
    */
    void VulkanSwapChain::createImageViews() 
    {
        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = swapChainImages[i];

            /**
             * The viewType and format fields specify how the image data should be interpreted. 
             * The viewType parameter allows you to treat images as 1D textures, 2D textures, 3D textures and cube maps.
            */
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = swapChainImageFormat;

            /**
             * The components field allows you to swizzle the color channels around. For example, you can map all of 
             * the channels to the red channel for a monochrome texture. You can also map constant values of 0 and 1 to a channel.
            */
            viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            /**
             * The subresourceRange field describes what the image's purpose is and which part of the image should be accessed. 
             * Our images will be used as color targets without any mipmapping levels or multiple layers.
            */
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateImageView(cyContext.getDevice()->device(), &viewInfo, nullptr, &swapChainImageViews[i]) == VK_SUCCESS, "Failed to create texture image view");
        }
    }

    void VulkanSwapChain::createRenderPass() 
    {
        /**
         * Creates a blueprint to be used for the framebuffer
        */
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachment{};
        //The format of the color attachment should match the format of the swap chain images
        colorAttachment.format = getSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

        /**
         * The loadOp and storeOp determine what to do with the data in the attachment before rendering and after rendering. We have the following choices for loadOp:
         * VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the attachment
         * VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at the start
         * VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined; we don't care about them
         * 
         * In our case we're going to use the clear operation to clear the framebuffer to black before drawing a new frame. There are only two possibilities for the storeOp:
         * VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in memory and can be read later
         * VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operation
        */
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

        /**
         * Textures and framebuffers in Vulkan are represented by VkImage objects with a certain pixel format, 
         * however the layout of the pixels in memory can change based on what you're trying to do with an image.
         * 
         * Some of the most common layouts are:
         * VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment
         * VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swap chain
         * VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Images to be used as destination for a memory copy operation
        */
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


        /**
         * A single render pass can consist of multiple subpasses. Subpasses are subsequent rendering operations that depend on 
         * the contents of framebuffers in previous passes, for example a sequence of post-processing effects that are applied one after another.
        */
        VkAttachmentReference colorAttachmentRef{};
        /**
         * The attachment parameter specifies which attachment to reference by its index in the attachment descriptions array. 
         * Our array consists of a single VkAttachmentDescription, so its index is 0. The layout specifies which layout we would like the 
         * attachment to have during a subpass that uses this reference. 
        */
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


        VkSubpassDescription subpass{};
        /**
         * The index of the attachment in this array is directly referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive!
         * The following other types of attachments can be referenced by a subpass:
         * 
         * pInputAttachments: Attachments that are read from a shader
         * pResolveAttachments: Attachments used for multisampling color attachments
         * pDepthStencilAttachment: Attachment for depth and stencil data
         * pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preserved 
        */
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        /**
         * he special value VK_SUBPASS_EXTERNAL refers to the implicit subpass before or after the render pass depending on whether 
         * it is specified in srcSubpass or dstSubpass. The index 0 refers to our subpass, which is the first and only one. The dstSubpass must 
         * always be higher than srcSubpass to prevent cycles in the dependency graph (unless one of the subpasses is VK_SUBPASS_EXTERNAL).
        */
        dependency.dstSubpass = 0;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        /**
         * The next two fields specify the operations to wait on and the stages in which these operations occur. 
         * We need to wait for the swap chain to finish reading from the image before we can access it. This can 
         * be accomplished by waiting on the color attachment output stage itself.
        */
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        
        dependency.srcAccessMask = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;


        /**
         * The render pass object can then be created by filling in the VkRenderPassCreateInfo structure with an array of 
         * attachments and subpasses. The VkAttachmentReference objects reference attachments using the indices of this array.
        */
        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateRenderPass(cyContext.getDevice()->device(), &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
    }

    void VulkanSwapChain::createIndexBuffers()
    {
        //can use either uint16_t or uint32_t
        //std::vector<uint16_t> indices =
        //{
        //    0, 1, 2, 2, 3, 0
        //};

        ////use reset here to ensure on recreation of the swap chain and of the vertex buffer that the previous index buffer is cleaned up
        //indexBuffer.reset(new VulkanIndexBuffer(cyContext, sizeof(indices[0]) * indices.size(), static_cast<void*>(indices.data())));
    }

    void VulkanSwapChain::createVertexBuffers()
    {
        std::vector<Vertex> vertices =
        {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
        };

        std::vector<uint16_t> indices =
        {
            0, 1, 2, 2, 3, 0
        };


        if(omniBuffer.get() != nullptr) omniBuffer->~VulkanBuffer();

        
        
        VkDeviceSize vSize = sizeof(vertices[0]) * vertices.size();
        VkDeviceSize iSize = sizeof(indices[0]) * indices.size();

        VulkanBuffer b(cyContext, vSize, vertices.data(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        omniBuffer.reset(new VulkanBuffer(cyContext, vSize, vertices.data(), iSize, indices.data()));
    }

    /**
     * @brief The attachments specified during render pass creation are bound by wrapping them into a VkFramebuffer object. 
     * A framebuffer object references all of the VkImageView objects that represent the attachments.
    */
    void VulkanSwapChain::createFramebuffers() 
    {
        swapChainFramebuffers.resize(imageCount());
        for (size_t i = 0; i < imageCount(); i++) 
        {
            std::array<VkImageView, 2> attachments = { swapChainImageViews[i], depthImageViews[i] };

            VkExtent2D swapChainExtent = getSwapChainExtent();
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;

            /**
             * The attachmentCount and pAttachments parameters specify the VkImageView objects that should be bound to the 
             * respective attachment descriptions in the render pass pAttachment array.
            */
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1; //layers refers to the number of layers in image arrays

            ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateFramebuffer(cyContext.getDevice()->device(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer");
        }
    }

    void VulkanSwapChain::createUniformBuffers()
    {
        uniformBuffers.resize(swapChainImages.size());
        BufferCreationAllocationInfo uniformBuffInfo = BufferCreationAllocationInfo::createDefaultUniformBuffer(static_cast<VkDeviceSize>(sizeof(UniformBufferObject)));
        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            uniformBuffers.push_back(std::move(VulkanBuffer(cyContext, uniformBuffInfo)));
        }
    }

    void VulkanSwapChain::createDepthResources() 
    {
        VkFormat depthFormat = findDepthFormat();
        VkExtent2D swapChainExtent = getSwapChainExtent();

        depthImages.resize(imageCount());
        depthImageMemorys.resize(imageCount());
        depthImageViews.resize(imageCount());

        for (int i = 0; i < depthImages.size(); i++) 
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = swapChainExtent.width;
            imageInfo.extent.height = swapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            cyContext.getDevice()->createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImages[i], depthImageMemorys[i]);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = depthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
            ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateImageView(cyContext.getDevice()->device(), &viewInfo, nullptr, &depthImageViews[i]) == VK_SUCCESS, "Failed to create texture image view");
        }
    }

    void VulkanSwapChain::createSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //create fence object with signaled = true

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            ASSERT_ERROR
            (
                DEFAULT_LOGGABLE, 
                vkCreateSemaphore(cyContext.getDevice()->device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) == VK_SUCCESS
                && vkCreateSemaphore(cyContext.getDevice()->device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) == VK_SUCCESS
                && vkCreateFence(cyContext.getDevice()->device(), &fenceInfo, nullptr, &inFlightFences[i]) == VK_SUCCESS, 
                "Failed to create synchronization objects for a frame"
            );
        }
    }

    /**
    * @brief You can use uniform values in shaders, which are globals similar to dynamic state variables that can be changed at drawing
    * time to alter the behavior of your shaders without having to recreate them. They are commonly used to pass the transformation matrix to the vertex shader,
    * or to create texture samplers in the fragment shader.
    * These uniform values need to be specified during pipeline creation by creating a VkPipelineLayout object.
    */
    void VulkanSwapChain::createDefaultPipelineLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional


        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;
        ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateDescriptorSetLayout(cyContext.getDevice()->device(), &layoutInfo, nullptr, &descriptorSetLayout) == VK_SUCCESS, "Failed to create descriptor set layout.");



        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; //used to send data to shaders
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreatePipelineLayout(cyContext.getDevice()->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout");
    }

    void VulkanSwapChain::createDefaultPipeline()
    {
        PipelineConfigInfo pipelineConfig{};
        VulkanPipeline::defaultPipelineConfigInfo(pipelineConfig, width(), height());
        pipelineConfig.renderPass = getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        cyPipeline = std::make_unique<VulkanPipeline>(cyContext, "src/resources/shaders/SimpleShader.vert.spv", "src/resources/shaders/SimpleShader.frag.spv", pipelineConfig);
    }

    /**
    * @brief Allocates and records the commands for each swap chain image.
    */
    void VulkanSwapChain::createCommandBuffers()
    {
        commandBuffers.resize(imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

        /**
         * The level parameter specifies if the allocated command buffers are primary or secondary command buffers.

         * VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
         * VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.
        */
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = cyContext.getDevice()->getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        ASSERT_ERROR(DEFAULT_LOGGABLE, vkAllocateCommandBuffers(cyContext.getDevice()->device(), &allocInfo, commandBuffers.data()) == VK_SUCCESS, "Failed to allocate command buffers");

       // VkBuffer vertexBuffers[] = { vertexBuffer->getVertexBuffer() };
        //VkDeviceSize offsets[] = { 0 };


        VkBuffer vertexBuffers[] = { omniBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };

        for (int i = 0; i < commandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            /**
             * The flags parameter specifies how we're going to use the command buffer. The following values are available:

             * VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
             * VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
             * VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
            */
            beginInfo.flags = 0; // Optional

            /**
             * The pInheritanceInfo parameter is only relevant for secondary command buffers.
             * It specifies which state to inherit from the calling primary command buffers.
            */
            beginInfo.pInheritanceInfo = nullptr; // Optional

            /**
             * If the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implicitly reset it.
             * It's not possible to append commands to a buffer at a later time.
            */
            ASSERT_ERROR(DEFAULT_LOGGABLE, vkBeginCommandBuffer(commandBuffers[i], &beginInfo) == VK_SUCCESS, "Failed  to begin recording command buffer");

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = getRenderPass();
            renderPassInfo.framebuffer = getFrameBuffer(i);

            /**
             * The render area defines where shader loads and stores will take place. The pixels outside this region
             * will have undefined values. It should match the size of the attachments for best performance.
            */
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
            clearValues[1].depthStencil = { 1.0f, 0 };
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            /**
             * The second parameter specifies if the pipeline object is a graphics or compute pipeline.
             * We've now told Vulkan which operations to execute in the graphics pipeline and which attachment to use in the fragment shader,
            */
            cyPipeline->bind(commandBuffers[i]);       

            /**
             * The vkCmdBindVertexBuffers function is used to bind vertex buffers to bindings, like the 
             * one we set up in the previous chapter. The first two parameters, besides the command buffer,
             * specify the offset and number of bindings we're going to specify vertex buffers for. The last
             * two parameters specify the array of vertex buffers to bind and the byte offsets to start reading 
             * vertex data from.
            */
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);


            /**
             * Parameters other than the command buffer are the index buffer, a byte offset into it, and the type of index data. 
             * The possible types are VK_INDEX_TYPE_UINT16 and VK_INDEX_TYPE_UINT32.
            */
            //vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer.get()->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);

            vkCmdBindIndexBuffer(commandBuffers[i], omniBuffer->getBuffer(), omniBuffer->offset(), VK_INDEX_TYPE_UINT16);

            /**
             * vkCmdDraw has the following parameters, aside from the command buffer:
             *
             * vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
             * instanceCount: Used for instanced rendering, use 1 if you're not doing that.
             * firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
             * firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
            */
            //vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertexBuffer.get()->size()), 1, 0, 0); 

            vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(omniBuffer->count()), 1, 0, 0, 0);

            vkCmdEndRenderPass(commandBuffers[i]);

            ASSERT_ERROR(DEFAULT_LOGGABLE, vkEndCommandBuffer(commandBuffers[i]) == VK_SUCCESS, "Failed to record command buffer.");
        }
    }

    /**
     * @brief Each VkSurfaceFormatKHR entry contains a format and a colorSpace member.
     * The format member specifies the color channels and types. For example, VK_FORMAT_B8G8R8A8_SRGB
     * means that we store the B, G, R and alpha channels in that order with an 8 bit unsigned integer for a total of 32 bits per pixel.
    */
    VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats) 
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }


    /**
     * There are four possible modes available in Vulkan:
     * 
     * VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred to the screen right away, which may result in tearing.
     * 
     * VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes an image from the front of the queue when the display is refreshed
     * and the program inserts rendered images at the back of the queue. If the queue is full then the program has to wait. This is most similar to vertical sync
     * as found in modern games. The moment that the display is refreshed is known as "vertical blank".

     * VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from the previous one if the application is late and the queue was empty at the last vertical blank.
     * Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives. This may result in visible tearing.

     * VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode. Instead of blocking the application when the queue is full, the images that are already
     * queued are simply replaced with the newer ones. This mode can be used to render frames as fast as possible while still avoiding tearing, resulting in fewer latency issues
     * than standard vertical sync. This is commonly known as "triple buffering", although the existence of three buffers alone does not necessarily mean that the framerate is unlocked.
     * 
     * NOTE: ONLY the VK_PRESENT_MODE_FIFO_KHR mode is guaranteed to be available.
     * 
    */
    VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        /**
         * Availible present modes are FIFO, Mailbox, and Immediate
         * 
         * FIFO is always supported and good for mobile
         * 
         * Mailbox has lower latency but is not always supported and has higher power consumption
         * 
         * Immediate has high power consumption, tearing, but offers low latency and usually is supported
         */
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                std::cout << "Present mode: Mailbox" << std::endl;
                return availablePresentMode;
            }
        }

        // for (const auto &availablePresentMode : availablePresentModes) {
        //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        //     std::cout << "Present mode: Immediate" << std::endl;
        //     return availablePresentMode;
        //   }
        // }

        std::cout << "Present mode: V-Sync" << std::endl;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    /**
     * @brief Vulkan works with pixels, so the swap chain extent must be specified in pixels as well.
     * if you are using a high DPI display (like Apple's Retina display), screen coordinates don't correspond to pixels.
     * Instead, due to the higher pixel density, the resolution of the window in pixel will be larger than the resolution in
     * screen coordinates. So if Vulkan doesn't fix the swap extent for us, we can't just use the original {WIDTH, HEIGHT}.
     * Instead, we must use glfwGetFramebufferSize to query the resolution of the window in pixel before matching it against
     * the minimum and maximum image extent.
     *
     * The clamp function is used here to bound the values of width and height between the allowed minimum and maximum extents that are supported by the implementation.
    */
    VkExtent2D VulkanSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        else 
        {
            VkExtent2D actualExtent = cyContext.getWindow()->getExtent();
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }

    VkFormat VulkanSwapChain::findDepthFormat()
    {
        return cyContext.getDevice()->findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

}