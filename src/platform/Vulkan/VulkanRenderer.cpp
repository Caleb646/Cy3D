#include "pch.h"

#include "VulkanRenderer.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"


namespace cy3d
{
	VulkanRenderer::VulkanRenderer(VulkanContext& context) : cyContext(context)
	{
		init();
	}

	VulkanRenderer::~VulkanRenderer()
	{
        cleanup();
	}

    void VulkanRenderer::cleanup()
    {
        vkFreeCommandBuffers(
            cyContext.getDevice()->device(), 
            cyContext.getDevice()->getCommandPool(), 
            static_cast<uint32_t>(commandBuffers.size()), 
            commandBuffers.data()
        );
    }

	void VulkanRenderer::beginFrame()
	{

        ASSERT_ERROR(DEFAULT_LOGGABLE, isFrameStarted == false, "Cannot begin a frame that hasnt ended.");
        VkResult res = cyContext.getSwapChain()->acquireNextImage(&currentImageIndex);

        /**
         * If the swap chain turns out to be out of date when attempting to acquire an image,
         * then it is no longer possible to present to it. Therefore we should immediately recreate
         * the swap chain and try again in the next drawFrame call.
        */
        if (res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else
        {
            ASSERT_ERROR(DEFAULT_LOGGABLE, res == VK_SUCCESS, "Failed to acquire image.");
        } 

        //needs to be after recreateSwapChain because if the window is resized it will cause 
        //the isFrameStart == false to fail
        isFrameStarted = true;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        ASSERT_ERROR(DEFAULT_LOGGABLE, 
            vkBeginCommandBuffer(getCurrentCommandBuffer(), &beginInfo) == VK_SUCCESS,
            "Failed to begin recording command buffer.");
	}

	void VulkanRenderer::endFrame()
	{
        ASSERT_ERROR(DEFAULT_LOGGABLE, isFrameStarted == true, "Cannot end a frame that hasnt started.");

        ASSERT_ERROR(DEFAULT_LOGGABLE,
            vkEndCommandBuffer(getCurrentCommandBuffer()) == VK_SUCCESS,
            "Failed to record command buffer.");

        VkResult res = cyContext.getSwapChain()->submitCommandBuffers(&getCurrentCommandBuffer(), &currentImageIndex);
        
        /**
         * If the swap chain turns out to be out of date when attempting to acquire an image,
         * then it is no longer possible to present to it. Therefore we should immediately recreate
         * the swap chain and try again in the next drawFrame call.
        */
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || cyContext.getWindow()->isWindowFrameBufferResized())
        {
            cyContext.getWindow()->resetWindowFrameBufferResizedFlag();
            recreateSwapChain();
        }

        else
        {
            ASSERT_ERROR(DEFAULT_LOGGABLE, res == VK_SUCCESS, "Failed to present swap chain image.");
        }

        isFrameStarted = false;

        std::size_t& currentFrameIndex = cyContext.getCurrentFrameIndex();
        currentFrameIndex = (currentFrameIndex + 1) % VulkanSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

    void VulkanRenderer::beginRenderPass(VkRenderPass& renderPass)
    {
        ASSERT_ERROR(DEFAULT_LOGGABLE, isFrameStarted == true, "Cannot end a frame that hasnt started.");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass; //cyContext.getSwapChain()->getRenderPass();
        renderPassInfo.framebuffer = cyContext.getSwapChain()->getFrameBuffer(currentImageIndex);

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = cyContext.getSwapChain()->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(getCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(cyContext.getSwapChain()->getSwapChainExtent().width);
        viewport.height = static_cast<float>(cyContext.getSwapChain()->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, cyContext.getSwapChain()->getSwapChainExtent() };
        vkCmdSetViewport(getCurrentCommandBuffer(), 0, 1, &viewport);
        vkCmdSetScissor(getCurrentCommandBuffer(), 0, 1, &scissor);
    }

    void VulkanRenderer::endRenderPass()
    {
        ASSERT_ERROR(DEFAULT_LOGGABLE, isFrameStarted == true, "Cannot end a frame that hasnt started.");

        vkCmdEndRenderPass(commandBuffers[cyContext.getCurrentFrameIndex()]);
    }

	void VulkanRenderer::init()
	{
		createCommandBuffers(); 
	}

    /**
    * @brief Allocates and records the commands for each swap chain image.
    */
	void VulkanRenderer::createCommandBuffers()
	{
        commandBuffers.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);

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
	}

    void VulkanRenderer::recreateSwapChain()
    {
        //if window is currently minimized block
        cyContext.getWindow()->blockWhileWindowMinimized();

        //make sure nothing is currently being used 
        //before cleaning up.
        vkDeviceWaitIdle(cyContext.getDevice()->device());

        cyContext.getSwapChain()->reCreate();

        cleanup();

        createCommandBuffers();

        _needsResize = true;
    }
}
