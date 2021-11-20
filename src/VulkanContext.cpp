#include "pch.h"

#include "VulkanContext.h"

#include "VulkanWindow.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"


namespace cy3d
{
	/**
	 * PUBLIC STATIC METHODS
	*/
	void VulkanContext::createDefaultContext(VulkanContext& emptyContext, WindowTraits wts)
	{
		emptyContext.cyWindow.reset(new VulkanWindow(wts));
		emptyContext.cyDevice.reset(new VulkanDevice(emptyContext));
		emptyContext.cySwapChain.reset(new VulkanSwapChain(emptyContext));
	}
}
