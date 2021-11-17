#include "FirstApp.h"

#include <array>

#include <Logi/Logi.h>

namespace cy3d
{
	FirstApp::FirstApp()
	{
		createPipelineLayout();
		createPipeline();
		createCommandBuffers();
	}
	FirstApp::~FirstApp()
	{
		vkDestroyPipelineLayout(cyDevice.device(), pipelineLayout, nullptr);
	}

	void FirstApp::run()
	{
		while (!cyWindow.shouldClose())
		{
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(cyDevice.device());
	}

	void FirstApp::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0; //used to send data to shaders
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreatePipelineLayout(cyDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout");
	}

	void FirstApp::createPipeline()
	{
		PipelineConfigInfo pipelineConfig{};
		CyPipeline::defaultPipelineConfigInfo(pipelineConfig, cySwapChain.width(), cySwapChain.height());
		pipelineConfig.renderPass = cySwapChain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		cyPipeline = std::make_unique<CyPipeline>(cyDevice, "src/resources/shaders/SimpleShader.vert.spv", "src/resources/shaders/SimpleShader.frag.spv", pipelineConfig);
	}

	void FirstApp::createCommandBuffers()
	{
		commandBuffers.resize(cySwapChain.imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = cyDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		ASSERT_ERROR(DEFAULT_LOGGABLE, vkAllocateCommandBuffers(cyDevice.device(), &allocInfo, commandBuffers.data()) == VK_SUCCESS, "Failed to allocate command buffers");

		for (int i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			ASSERT_ERROR(DEFAULT_LOGGABLE, vkBeginCommandBuffer(commandBuffers[i], &beginInfo) == VK_SUCCESS, "Failed  to begin recording command buffer");

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = cySwapChain.getRenderPass();
			renderPassInfo.framebuffer = cySwapChain.getFrameBuffer(i);

			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = cySwapChain.getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			cyPipeline->bind(commandBuffers[i]);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}

	void FirstApp::drawFrame()
	{
		uint32_t imageIndex;
		auto result = cySwapChain.acquireNextImage(&imageIndex);
		ASSERT_ERROR(DEFAULT_LOGGABLE, result == VK_SUCCESS, "Failed to acquire swap chain image");
		result = cySwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		ASSERT_ERROR(DEFAULT_LOGGABLE, result == VK_SUCCESS, "Failed to present swap chain image");
	}
}



