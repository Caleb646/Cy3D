#include "pch.h"

#include "FirstApp.h"

#include <Logi/Logi.h>

namespace cy3d
{
	FirstApp::FirstApp() 
	{
		VulkanContext::createDefaultContext(cyContext);
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run()
	{
		while (!cyContext.getWindow()->shouldClose())
		{
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(cyContext.getDevice()->device());
	}

	/**
	 * @brief The drawFrame function will perform the following operations:
	 *
	 * Acquire an image from the swap chain
	 * Execute the command buffer with that image as attachment in the framebuffer
	 * Return the image to the swap chain for presentation
	 * 
	 * Each of these events is set in motion using a single function call, but they are executed asynchronously. 
	 * The function calls will return before the operations are actually finished and the order of execution is also undefined.
	*/
	void FirstApp::drawFrame()
	{
		uint32_t imageIndex;
		auto result = cyContext.getSwapChain()->acquireNextImage(&imageIndex);
		//ASSERT_ERROR(DEFAULT_LOGGABLE, result == VK_SUCCESS, "Failed to acquire swap chain image");
		result = cyContext.getSwapChain()->submitCommandBuffers(&imageIndex);
		//ASSERT_ERROR(DEFAULT_LOGGABLE, result == VK_SUCCESS, "Failed to present swap chain image");
	}
}



