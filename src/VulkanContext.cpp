#include "pch.h"

#include "VulkanContext.h"

#include "CyWindow.h"
#include "CyDevice.h"
#include "CySwapChain.h"


namespace cy3d
{
	/**
	 * PUBLIC STATIC METHODS
	*/
	void VulkanContext::createDefaultContext(VulkanContext& emptyContext, WindowTraits wts = WindowTraits{ 800, 600, "Hello" })
	{
		emptyContext.cyWindow.reset(new CyWindow(wts));
		emptyContext.cyDevice.reset(new CyDevice(emptyContext));
		emptyContext.cySwapChain.reset(new CySwapChain(emptyContext));
	}
}
