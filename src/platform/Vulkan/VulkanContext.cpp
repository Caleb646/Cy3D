#include "pch.h"

#include "VulkanContext.h"

#include "VulkanWindow.h"
#include "VulkanDevice.h"
#include "VulkanAllocator.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderer.h"
#include "../../src/ShaderManager.h"



namespace cy3d
{
	std::size_t& VulkanContext::getCurrentFrameIndex()
	{
		return currentFrameIndex;
	}

	uint32_t VulkanContext::getWindowWidth()
	{
		CY_ASSERT(cySwapChain.get() != nullptr);
		return cySwapChain->getWidth();
	}

	uint32_t VulkanContext::getWindowHeight()
	{
		CY_ASSERT(cySwapChain.get() != nullptr);
		return cySwapChain->getHeight();
	}

	auto VulkanContext::getWindowExtent()
	{
		CY_ASSERT(cySwapChain.get() != nullptr);
		return cySwapChain->getSwapChainExtent();
	}

	VulkanWindow* VulkanContext::getWindow()
	{
		CY_ASSERT(cyWindow.get() != nullptr);
		return cyWindow.get();
	}

	VulkanDevice* VulkanContext::getDevice()
	{
		CY_ASSERT(cyDevice.get() != nullptr);
		return cyDevice.get();
	}

	VulkanAllocator* VulkanContext::getAllocator()
	{
		CY_ASSERT(vulkanAllocator.get() != nullptr);
		return vulkanAllocator.get();
	}

	VulkanSwapChain* VulkanContext::getSwapChain()
	{
		CY_ASSERT(cySwapChain.get() != nullptr);
		return cySwapChain.get();
	}

	VulkanRenderer* VulkanContext::getVulkanRenderer()
	{
		CY_ASSERT(vulkanRenderer.get() != nullptr);
		return vulkanRenderer.get();
	}

	Ref<ShaderManager> VulkanContext::getShaderManager()
	{
		CY_ASSERT(shaderManager.get() != nullptr);
		return shaderManager;
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
		emptyContext.shaderManager.reset(new ShaderManager(emptyContext));
	}
}
