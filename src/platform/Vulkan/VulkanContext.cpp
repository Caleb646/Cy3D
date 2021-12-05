#include "pch.h"

#include "VulkanContext.h"

#include <Logi/Logi.h>

#include "VulkanWindow.h"
#include "VulkanDevice.h"
#include "VulkanAllocator.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderer.h"


namespace cy3d
{
	std::size_t& VulkanContext::getCurrentFrameIndex()
	{
		return currentFrameIndex;
	}

	uint32_t VulkanContext::getWindowWidth()
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, cySwapChain.get() != nullptr, "SwapChain ptr is null.");
		return cySwapChain->getWidth();
	}

	uint32_t VulkanContext::getWindowHeight()
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, cySwapChain.get() != nullptr, "SwapChain ptr is null.");
		return cySwapChain->getHeight();
	}

	auto VulkanContext::getWindowExtent()
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, cySwapChain.get() != nullptr, "SwapChain ptr is null.");
		return cySwapChain->getSwapChainExtent();
	}

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

	VulkanRenderer* VulkanContext::getVulkanRenderer()
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, vulkanRenderer.get() != nullptr, "VulkanRenderer ptr is null.");
		return vulkanRenderer.get();
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
		emptyContext.vulkanRenderer.reset(new VulkanRenderer(emptyContext));
	}
}
