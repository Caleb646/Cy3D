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

        CY_ASSERT(isFrameStarted == false);
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
            CY_ASSERT(res == VK_SUCCESS);
        } 

        //needs to be after recreateSwapChain because if the window is resized it will cause 
        //the isFrameStart == false to fail
        isFrameStarted = true;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CHECK(vkBeginCommandBuffer(getCurrentCommandBuffer(), &beginInfo));
	}

	void VulkanRenderer::endFrame()
	{
        CY_ASSERT(isFrameStarted == true);
        VK_CHECK(vkEndCommandBuffer(getCurrentCommandBuffer()));

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
            CY_ASSERT(res == VK_SUCCESS);
        }

        isFrameStarted = false;

        std::size_t& currentFrameIndex = cyContext.getCurrentFrameIndex();
        currentFrameIndex = (currentFrameIndex + 1) % VulkanSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

    void VulkanRenderer::beginRenderPass(VkRenderPass& renderPass)
    {
        CY_ASSERT(isFrameStarted == true);

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
        CY_ASSERT(isFrameStarted == true);

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

        VK_CHECK(vkAllocateCommandBuffers(cyContext.getDevice()->device(), &allocInfo, commandBuffers.data()));
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
