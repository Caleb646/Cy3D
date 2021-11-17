#pragma once

#include "pch.h"

#include "CyDevice.h"

namespace cy3d
{
	struct PipelineConfigInfo
	{
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class CyPipeline
	{
	private:
		CyDevice& cyDevice;
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

	public:
		CyPipeline(CyDevice& d, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& config);
		~CyPipeline();

		//delete copy methods
		CyPipeline(const CyPipeline&) = delete;
		CyPipeline& operator=(const CyPipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);

		/*
		* PUBLIC STATIC METHODS
		*/
		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo, uint32_t width, uint32_t height);

	private:
		void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& config);

		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		/*
		* PRIVATE STATIC METHODS
		*/
		static std::vector<char> readFile(const std::string& filename);
	};
}


