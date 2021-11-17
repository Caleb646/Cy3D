#pragma once
#include "pch.h"

#include "CyPipeline.h"
#include "CyWindow.h"
#include "CyDevice.h"
#include "CySwapChain.h"

namespace cy3d
{
	class FirstApp
	{
	private:
		//are instantiated from top to bottom and destroyed from bottom to top.
		CyWindow cyWindow{ WindowTraits{ 800, 600, "Hello" } };
		CyDevice cyDevice{ cyWindow, };
		CySwapChain cySwapChain{ cyDevice, cyWindow.getExtent() };
		std::unique_ptr<CyPipeline> cyPipeline; // {cyDevice, "src/resources/shaders/SimpleShader.vert", "src/resources/shaders/SimpleShader.frag", CyPipeline::defaultPipelineConfigInfo(800, 600) };
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;

	public:
		FirstApp();
		~FirstApp();

		//delete copy methods
		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void run();

	private:
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();
	};

}


