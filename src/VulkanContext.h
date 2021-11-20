#pragma once
#include "pch.h"

#include <Logi/Logi.h>

#include "Fwd.hpp"
#include "VulkanWindow.h"

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
		std::unique_ptr<VulkanWindow> cyWindow{ nullptr };
		std::unique_ptr<VulkanDevice> cyDevice{ nullptr };
		std::unique_ptr<VulkanSwapChain> cySwapChain{ nullptr };

	public:
		VulkanContext() = default;

		//no copy assignment, move, or constructor
		VulkanContext(const VulkanContext&) = delete;
		VulkanContext(VulkanContext&&) noexcept = delete;
		VulkanContext& operator=(const VulkanContext&) = delete;
		
		VulkanWindow* getWindow() 
		{ 
			ASSERT_ERROR(DEFAULT_LOGGABLE, cyWindow.get() != nullptr, "Window ptr is null.");
			return cyWindow.get();
		}
		VulkanDevice* getDevice() 
		{ 
			ASSERT_ERROR(DEFAULT_LOGGABLE, cyDevice.get() != nullptr, "Device ptr is null.");
			return cyDevice.get();
		}
		VulkanSwapChain* getSwapChain()
		{ 
			ASSERT_ERROR(DEFAULT_LOGGABLE, cySwapChain.get() != nullptr, "SwapChain ptr is null.");
			return cySwapChain.get();
		}

		/**
		 * PUBLIC STATIC METHODS
		*/
		static void createDefaultContext(VulkanContext& emptyContext, WindowTraits wts = WindowTraits{ 800, 600, "Hello" });
	};
}



