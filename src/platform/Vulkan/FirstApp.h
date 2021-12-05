#pragma once
#include "pch.h"

#include "VulkanPipeline.h"
#include "VulkanWindow.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderer.h"
#include "VulkanContext.h"

namespace cy3d
{
	class FirstApp
	{
	private:
		//are instantiated from top to bottom and destroyed from bottom to top.
		//VulkanWindow cyWindow{ WindowTraits{ 800, 600, "Hello" } };
		//VulkanDevice cyDevice{ cyWindow, };
		//VulkanSwapChain cySwapChain{ cyDevice, cyWindow };
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

