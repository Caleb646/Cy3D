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

        //_descPool->resetDescriptors();

        vkDestroyPipelineLayout(cyContext.getDevice()->device(), _pipelineLayout, nullptr);
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

        //FOR TESTING ONLY
        UniformBufferObject ubo{};
        ubo.update(cyContext.getSwapChain()->getWidth(), cyContext.getSwapChain()->getHeight());
        mvpUbos[currentImageIndex]->setData(&ubo, 0);
        //END

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

    void VulkanRenderer::beginRenderPass()
    {
        ASSERT_ERROR(DEFAULT_LOGGABLE, isFrameStarted == true, "Cannot end a frame that hasnt started.");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = cyContext.getSwapChain()->getRenderPass();
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
        //vkCmdSetViewport(getCurrentCommandBuffer(), 0, 1, &viewport);
        //vkCmdSetScissor(getCurrentCommandBuffer(), 0, 1, &scissor);




        //FOR Testing only

        VkBuffer vertexBuffers[] = { _omniBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };

      

        /**
            * The second parameter specifies if the pipeline object is a graphics or compute pipeline.
            * We've now told Vulkan which operations to execute in the graphics pipeline and which attachment to use in the fragment shader,
        */
        _cyPipeline->bind(getCurrentCommandBuffer());
        vkCmdBindDescriptorSets(getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _cyPipeline->getPipelineLayout(), 0, 1, &_descSets->at(cyContext.getCurrentFrameIndex()), 0, nullptr);

        /**
            * The vkCmdBindVertexBuffers function is used to bind vertex buffers to bindings, like the
            * one we set up in the previous chapter. The first two parameters, besides the command buffer,
            * specify the offset and number of bindings we're going to specify vertex buffers for. The last
            * two parameters specify the array of vertex buffers to bind and the byte offsets to start reading
            * vertex data from.
        */
        vkCmdBindVertexBuffers(getCurrentCommandBuffer(), 0, 1, vertexBuffers, offsets);


        /**
            * Parameters other than the command buffer are the index buffer, a byte offset into it, and the type of index data.
            * The possible types are VK_INDEX_TYPE_UINT16 and VK_INDEX_TYPE_UINT32.
        */
        //vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer.get()->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);

        vkCmdBindIndexBuffer(getCurrentCommandBuffer(), _omniBuffer->getBuffer(), _omniBuffer->offset(), VK_INDEX_TYPE_UINT16);

        /**
            * vkCmdDraw has the following parameters, aside from the command buffer:
            *
            * vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
            * instanceCount: Used for instanced rendering, use 1 if you're not doing that.
            * firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
            * firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
        */
        //vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertexBuffer.get()->size()), 1, 0, 0); 

        


        vkCmdDrawIndexed(getCurrentCommandBuffer(), static_cast<uint32_t>(_omniBuffer->count()), 1, 0, 0, 0);

        //vkCmdEndRenderPass(commandBuffers[i]);

        //ASSERT_ERROR(DEFAULT_LOGGABLE, vkEndCommandBuffer(commandBuffers[i]) == VK_SUCCESS, "Failed to record command buffer.");


        //END



    }

    void VulkanRenderer::endRenderPass()
    {
        ASSERT_ERROR(DEFAULT_LOGGABLE, isFrameStarted == true, "Cannot end a frame that hasnt started.");

        vkCmdEndRenderPass(commandBuffers[cyContext.getCurrentFrameIndex()]);
    }

	void VulkanRenderer::init()
	{
		createUniformBuffers();
		createDefaultPipelineLayout();
		
		createDescriptorPools();
        createDefaultPipeline();
		createDescriptorSets();

        createTestVertices();

		createCommandBuffers(); 
	}

	void VulkanRenderer::createUniformBuffers()
	{
        mvpUbos.resize(cyContext.getSwapChain()->imageCount());
        BufferCreationAllocationInfo uniformBuffInfo = BufferCreationAllocationInfo::createDefaultUniformBuffer(static_cast<VkDeviceSize>(sizeof(UniformBufferObject)));
        for (size_t i = 0; i < cyContext.getSwapChain()->imageCount(); i++)
        {
            mvpUbos[i].reset(new VulkanBuffer(cyContext, uniformBuffInfo, 1));
        }
	}

	void VulkanRenderer::createDefaultPipelineLayout()
	{
        _descLayout.reset(new VulkanDescriptorSetLayout(cyContext));
        _descLayout->
             addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        //VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        //pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        //pipelineLayoutInfo.setLayoutCount = 1;
        //pipelineLayoutInfo.pSetLayouts = &_descLayout->getLayout();
        //pipelineLayoutInfo.pushConstantRangeCount = 0; //used to send data to shaders
        //pipelineLayoutInfo.pPushConstantRanges = nullptr;
        //ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreatePipelineLayout(cyContext.getDevice()->device(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout");
	}
	void VulkanRenderer::createDefaultPipeline()
	{
        //PipelineConfigInfo pipelineConfig{};
        //auto width = cyContext.getSwapChain()->getWidth();
        //auto height = cyContext.getSwapChain()->getHeight();
        //VulkanPipeline::defaultPipelineConfigInfo(pipelineConfig, width, height);


        //set up pipeline
        PipelineConfigInfo pipelineConfig{};
        VulkanPipeline::defaultPipelineConfigInfo(pipelineConfig, cyContext.getWindowWidth(), cyContext.getWindowHeight());
        pipelineConfig.renderPass = cyContext.getSwapChain()->getRenderPass();
        PipelineLayoutConfigInfo layoutInfo(&_descLayout->getLayout());
        _cyPipeline.reset(new VulkanPipeline(cyContext, pipelineConfig, layoutInfo));

        //pipelineConfig.renderPass = cyContext.getSwapChain()->getRenderPass();
        //pipelineConfig.pipelineLayout = getPipelineLayout();
        //_cyPipeline.reset(new VulkanPipeline(cyContext, pipelineConfig));
	}

	void VulkanRenderer::createDescriptorPools()
	{
        uint32_t numImages = static_cast<uint32_t>(cyContext.getSwapChain()->imageCount());
        _descPool.reset(new VulkanDescriptorPool(cyContext, 
            { 
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numImages },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numImages }
            }
        ));

	}
	void VulkanRenderer::createDescriptorSets()
	{
        uint32_t numImages = static_cast<uint32_t>(cyContext.getSwapChain()->imageCount());

        _texture.reset(new VulkanTexture(cyContext, "src/resources/textures/viking_room.png"));
        _descSets.reset(new VulkanDescriptorSets(cyContext, _descPool.get(), _descLayout.get(), numImages));

        for (std::size_t i = 0; i < numImages; i++)
        {
            _descSets->writeBufferToSet(mvpUbos[i]->descriptorBufferInfo(), i, 0);
            _descSets->writeImageToSet(_texture->descriptorImageInfo(), i, 1);
        }
        _descSets->updateSets();
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

        // VkBuffer vertexBuffers[] = { vertexBuffer->getVertexBuffer() };
         //VkDeviceSize offsets[] = { 0 };


        //VkBuffer vertexBuffers[] = { _omniBuffer->getBuffer() };
        //VkDeviceSize offsets[] = { 0 };

        //for (int i = 0; i < commandBuffers.size(); i++)
        //{
        //    VkCommandBufferBeginInfo beginInfo{};
        //    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        //    /**
        //     * The flags parameter specifies how we're going to use the command buffer. The following values are available:

        //     * VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
        //     * VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
        //     * VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
        //    */
        //    beginInfo.flags = 0; // Optional

        //    /**
        //     * The pInheritanceInfo parameter is only relevant for secondary command buffers.
        //     * It specifies which state to inherit from the calling primary command buffers.
        //    */
        //    beginInfo.pInheritanceInfo = nullptr; // Optional

        //    /**
        //     * If the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implicitly reset it.
        //     * It's not possible to append commands to a buffer at a later time.
        //    */
        //    ASSERT_ERROR(DEFAULT_LOGGABLE, vkBeginCommandBuffer(commandBuffers[i], &beginInfo) == VK_SUCCESS, "Failed  to begin recording command buffer");

        //    VkRenderPassBeginInfo renderPassInfo{};
        //    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        //    renderPassInfo.renderPass = cyContext.getSwapChain()->getRenderPass();
        //    renderPassInfo.framebuffer = cyContext.getSwapChain()->getFrameBuffer(i);

        //    /**
        //     * The render area defines where shader loads and stores will take place. The pixels outside this region
        //     * will have undefined values. It should match the size of the attachments for best performance.
        //    */
        //    renderPassInfo.renderArea.offset = { 0, 0 };
        //    renderPassInfo.renderArea.extent = cyContext.getSwapChain()->getSwapChainExtent();

        //    std::array<VkClearValue, 2> clearValues{};
        //    clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
        //    clearValues[1].depthStencil = { 1.0f, 0 };
        //    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        //    renderPassInfo.pClearValues = clearValues.data();

        //    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        //    /**
        //     * The second parameter specifies if the pipeline object is a graphics or compute pipeline.
        //     * We've now told Vulkan which operations to execute in the graphics pipeline and which attachment to use in the fragment shader,
        //    */
        //    _cyPipeline->bind(commandBuffers[i]);

        //    /**
        //     * The vkCmdBindVertexBuffers function is used to bind vertex buffers to bindings, like the
        //     * one we set up in the previous chapter. The first two parameters, besides the command buffer,
        //     * specify the offset and number of bindings we're going to specify vertex buffers for. The last
        //     * two parameters specify the array of vertex buffers to bind and the byte offsets to start reading
        //     * vertex data from.
        //    */
        //    vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);


        //    /**
        //     * Parameters other than the command buffer are the index buffer, a byte offset into it, and the type of index data.
        //     * The possible types are VK_INDEX_TYPE_UINT16 and VK_INDEX_TYPE_UINT32.
        //    */
        //    //vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer.get()->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);

        //    vkCmdBindIndexBuffer(commandBuffers[i], _omniBuffer->getBuffer(), _omniBuffer->offset(), VK_INDEX_TYPE_UINT16);

        //    /**
        //     * vkCmdDraw has the following parameters, aside from the command buffer:
        //     *
        //     * vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
        //     * instanceCount: Used for instanced rendering, use 1 if you're not doing that.
        //     * firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
        //     * firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
        //    */
        //    //vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertexBuffer.get()->size()), 1, 0, 0); 

        //    vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descSets->at(i), 0, nullptr);


        //    vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(_omniBuffer->count()), 1, 0, 0, 0);

        //    vkCmdEndRenderPass(commandBuffers[i]);

        //    ASSERT_ERROR(DEFAULT_LOGGABLE, vkEndCommandBuffer(commandBuffers[i]) == VK_SUCCESS, "Failed to record command buffer.");
        //}
	}

    void VulkanRenderer::createTestVertices()
    {
        std::vector<Vertex> vertices =
        {
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
        };

        std::vector<uint16_t> indices =
        {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
        };

        VkDeviceSize vSize = sizeof(vertices[0]) * vertices.size();
        VkDeviceSize iSize = sizeof(indices[0]) * indices.size();

        _omniBuffer.reset(new VulkanBuffer(cyContext, vSize, vertices.data(), iSize, indices.data()));
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

        createDefaultPipeline();
        createCommandBuffers();

        _needsResize = true;
    }
}
