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

		std::vector<std::unique_ptr<VulkanBuffer>> mvpUbos;
		/**
		 * Record a command buffer for every image in the swap chain.
		*/
		std::vector<VkCommandBuffer> commandBuffers;
		VkPipelineLayout _pipelineLayout;
		std::unique_ptr<VulkanPipeline> _cyPipeline;
		//descriptor sets
		std::unique_ptr<VulkanDescriptorPool> _descPool;
		std::unique_ptr<VulkanDescriptorSetLayout> _descLayout;
		std::unique_ptr<VulkanDescriptorSets> _descSets;
		//buffers
		std::unique_ptr<VulkanBuffer> _omniBuffer;
		//textures
		std::unique_ptr<VulkanTexture> _texture;

		uint32_t currentImageIndex{};
		bool isFrameStarted{ false };
		bool _needsResize{ false };


	public:
		VulkanRenderer(VulkanContext& context);
		~VulkanRenderer();
		void cleanup();
		void beginFrame();
		void endFrame();
		void beginRenderPass();
		void endRenderPass();

		void resetNeedsResize() { _needsResize = false; }
		bool needsResize() { return _needsResize; }
		VkCommandBuffer& getCurrentCommandBuffer()
		{
			ASSERT_ERROR(DEFAULT_LOGGABLE, isFrameStarted == true, "Cannot get a framebuffer when a frame hasnt begin.");
			return commandBuffers[cyContext.getCurrentFrameIndex()];
		}

	private:
		void init();
		void createUniformBuffers();
		void createDefaultPipelineLayout();
		void createDefaultPipeline();
		void createDescriptorPools();
		void createDescriptorSets();
		void createCommandBuffers();


		void createTestVertices();


		void recreateSwapChain();

		//getters
		VkPipelineLayout& getPipelineLayout() { return _pipelineLayout; }
	};
}



