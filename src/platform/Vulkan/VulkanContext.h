#pragma once
#include "pch.h"

#include "../../core/core.h"

#include "Vulkan.h"
#include "VulkanWindow.h"
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
		std::size_t currentFrameIndex{ 0 };
		std::unique_ptr<VulkanWindow> cyWindow{ nullptr };
		std::unique_ptr<VulkanDevice> cyDevice{ nullptr };
		std::unique_ptr<VulkanAllocator> vulkanAllocator{ nullptr };
		std::unique_ptr<VulkanSwapChain> cySwapChain{ nullptr };
		std::unique_ptr<VulkanRenderer> vulkanRenderer{ nullptr };
		Ref<ShaderManager> shaderManager{ nullptr };

	public:
		VulkanContext() = default;

		//no copy assignment, move, or constructor
		VulkanContext(const VulkanContext&) = delete;
		VulkanContext(VulkanContext&&) noexcept = delete;
		VulkanContext& operator=(const VulkanContext&) = delete;
		

		std::size_t& getCurrentFrameIndex();
		uint32_t getWindowWidth();
		uint32_t getWindowHeight();
		auto getWindowExtent();
		VulkanWindow* getWindow();
		VulkanDevice* getDevice();
		VulkanAllocator* getAllocator();
		VulkanSwapChain* getSwapChain();
		VulkanRenderer* getVulkanRenderer();

		Ref<ShaderManager> getShaderManager();

		/**
		 * PUBLIC STATIC METHODS
		*/
		static void createDefaultContext(VulkanContext& emptyContext, WindowTraits wts = WindowTraits{ 800, 600, "Hello" });
	};
}



