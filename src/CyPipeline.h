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


		/**
		 * A viewport basically describes the region of the framebuffer that the output will be rendered to. This will almost always be (0, 0) to (width, height) and 
		 * in this tutorial that will also be the case.
		*/
		VkViewport viewport;

		VkRect2D scissor;
		VkPipelineViewportStateCreateInfo viewportInfo;

		/**
		 * The VkPipelineInputAssemblyStateCreateInfo struct describes two things: what kind of geometry will be drawn from the vertices and if primitive
		 * restart should be enabled. The former is specified in the topology member and can have values like:

		 * VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
		 * VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse
		 * VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is used as start vertex for the next line
		 * VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse
		 * VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle
		 * 
		 * Normally, the vertices are loaded from the vertex buffer by index in sequential order, but with an element buffer you can specify the indices to use yourself. 
		 * This allows you to perform optimizations like reusing vertices. If you set the primitiveRestartEnable member to VK_TRUE, then it's possible to break up lines and triangles in the _STRIP 
		 * topology modes by using a special index of 0xFFFF or 0xFFFFFFFF.
		*/
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
		VkPipeline getGraphicsPipeline() { return graphicsPipeline; }
		/*
		* PUBLIC STATIC METHODS
		*/
		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo, uint32_t width, uint32_t height);
		static void defaultPipelineLayout(VkPipelineLayout& defaultLayout);

	private:

		void cleanup();

		void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& config);
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		/*
		* PRIVATE STATIC METHODS
		*/
		static std::vector<char> readFile(const std::string& filename);
	};
}


