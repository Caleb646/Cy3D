#include "pch.h"

#include "VulkanContext.h"

#include <Logi/Logi.h>

#include "VulkanWindow.h"
#include "VulkanDevice.h"
#include "VulkanAllocator.h"
#include "VulkanSwapChain.h"


namespace cy3d
{
	VulkanWindow* VulkanContext::getWindow()
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, cyWindow.get() != nullptr, "Window ptr is null.");
		return cyWindow.get();
	}

	VulkanDevice* VulkanContext::getDevice()
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, cyDevice.get() != nullptr, "Device ptr is null.");
		return cyDevice.get();
	}

	VulkanAllocator* VulkanContext::getAllocator()
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, vulkanAllocator.get() != nullptr, "Allocator ptr is null.");
		return vulkanAllocator.get();
	}

	VulkanSwapChain* VulkanContext::getSwapChain()
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, cySwapChain.get() != nullptr, "SwapChain ptr is null.");
		return cySwapChain.get();
	}

	/**
	 * PUBLIC STATIC METHODS
	*/
	void VulkanContext::createDefaultContext(VulkanContext& emptyContext, WindowTraits wts)
	{
		emptyContext.cyWindow.reset(new VulkanWindow(wts));
		emptyContext.cyDevice.reset(new VulkanDevice(emptyContext));
		emptyContext.vulkanAllocator.reset(new VulkanAllocator(emptyContext));
		emptyContext.cySwapChain.reset(new VulkanSwapChain(emptyContext));
	}
}
