#pragma once
#include "pch.h"

#include "CyPipeline.h"
#include "CyWindow.h"
#include "CyDevice.h"
#include "CySwapChain.h"
#include "VulkanContext.h"

namespace cy3d
{
	class FirstApp
	{
	private:
		//are instantiated from top to bottom and destroyed from bottom to top.
		//CyWindow cyWindow{ WindowTraits{ 800, 600, "Hello" } };
		//CyDevice cyDevice{ cyWindow, };
		//CySwapChain cySwapChain{ cyDevice, cyWindow };
		VulkanContext cyContext; 

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

		friend class VulkanContext;
	};

}


