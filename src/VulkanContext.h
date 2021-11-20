#pragma once
#include "pch.h"

#include "Fwd.hpp"

namespace cy3d
{
	/**
	 * @brief Is responsible for creating unique instances of cyWindow, cyDevice, and cySwapChain. Cannot be
	 * copied, assigned, or moved. Is not a singleton class. Multiple context objects can be created but they will all
	 * have their own unique cyWindow, cyDevice, and cySwapChain.
	*/
	class VulkanContext
	{
	private:
		std::unique_ptr<CyWindow> cyWindow;
		std::unique_ptr<CyDevice> cyDevice;
		std::unique_ptr<CySwapChain> cySwapChain;

	public:
		VulkanContext() = default;

		//no copy assignment, move, or constructor
		VulkanContext(const VulkanContext&) = delete;
		VulkanContext(VulkanContext&&) noexcept = delete;
		VulkanContext& operator=(const VulkanContext&) = delete;
		
		CyWindow* getWindow() { return cyWindow.get(); }
		CyDevice* getDevice() { return cyDevice.get(); }
		CySwapChain* getSwapChain() { return cySwapChain.get(); }

		/**
		 * PUBLIC STATIC METHODS
		*/
		static void createDefaultContext(VulkanContext& emptyContext, WindowTraits wts);
	};
}



