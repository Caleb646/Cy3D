#pragma once
#include "pch.h"

#include <Logi/Logi.h>

#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanDescriptors.h"
#include "VulkanTexture.h"

namespace cy3d
{
	class VulkanRenderer
	{

	private:
		VulkanContext& cyContext;
		/**
		 * Record a command buffer for every image in the swap chain.
		*/
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex{};
		bool isFrameStarted{ false };
		bool _needsResize{ false };


	public:
		VulkanRenderer(VulkanContext& context);
		~VulkanRenderer();
		void cleanup();
		void beginFrame();
		void endFrame();
		void beginRenderPass(VkRenderPass& renderPass);
		void endRenderPass();

		void resetNeedsResize() { _needsResize = false; }
		bool needsResize() { return _needsResize; }
		VkCommandBuffer& getCurrentCommandBuffer()
		{
			ASSERT_ERROR(DEFAULT_LOGGABLE, isFrameStarted == true, "Cannot get a command buffer when the frame hasnt began.");
			return commandBuffers[cyContext.getCurrentFrameIndex()];
		}
		uint32_t getCurrentImageIndex()
		{
			ASSERT_ERROR(DEFAULT_LOGGABLE, isFrameStarted == true, "Cannot get the current image index when the frame hasnt began.");
			return currentImageIndex;
		}

	private:
		void init();
		void createCommandBuffers();


		void recreateSwapChain();

	};
}



