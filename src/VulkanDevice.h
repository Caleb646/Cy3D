#pragma once

#include "pch.h"

#include "VulkanWindow.h"
#include "Fwd.hpp"

namespace cy3d
{

	struct BufferCreationAllocationInfo
	{
		VkDeviceSize size;
		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags properties;
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices
	{
		//std::optional variables on assignment of value will return true for has_value()
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	class VulkanDevice
	{

	public:
		VkPhysicalDeviceProperties properties;
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif

	private:
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		/**
		 * The physical graphics card that will be used.
		 * will be destroyed when VkInstance is destroyed.
		*/
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		//VulkanWindow& window;

		VulkanContext& cyContext;

		/**
		 * Command pools manage the memory that is used to store the buffers and 
		 * command buffers are allocated from them.
		*/
		VkCommandPool commandPool;

		/**
		 * a pointer to store the logical device handle in
		*/
		VkDevice device_;

		/**
		 * The VK_KHR_surface extension is an instance level extension and we've actually already enabled it, 
		 * because it's included in the list returned by glfwGetRequiredInstanceExtensions.
		 * 
		 * Although the VkSurfaceKHR object and its usage is platform agnostic, its creation 
		 * isn't because it depends on window system details. For example, it needs the HWND 
		 * and HMODULE handles on Windows. Therefore there is a platform-specific addition to the 
		 * extension, which on Windows is called VK_KHR_win32_surface and is also automatically 
		 * included in the list from glfwGetRequiredInstanceExtensions.
		*/
		VkSurfaceKHR surface_;

		/**
		 * The queues are automatically created along with the logical device, but we don't have a handle to interface with them yet. 
		 * First add a class member to store a handle to the graphics and presents queues:
		*/
		VkQueue graphicsQueue_;
		VkQueue presentQueue_;

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		//
		/**
		* The supported features/extensions of physicalDevice.
		* 
		* Check that the physical device has Swap Chain Support
		*/
		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };


	public:
		//VulkanDevice(VulkanWindow& window);
		VulkanDevice(VulkanContext& context);
		~VulkanDevice();

		// Not copyable or movable
		VulkanDevice(const VulkanDevice&) = delete;
		void operator=(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		VulkanDevice& operator=(VulkanDevice&&) = delete;

		VkCommandPool getCommandPool() { return commandPool; }
		VkDevice device() { return device_; }
		VkSurfaceKHR surface() { return surface_; }
		VkQueue graphicsQueue() { return graphicsQueue_; }
		VkQueue presentQueue() { return presentQueue_; }

		SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		// Buffer Helper Functions
		void createBuffer(BufferCreationAllocationInfo cyBufferInfo, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void fillBuffer(void* dataToCopy, VkDeviceSize size, VkDeviceMemory& bufferMemory);	
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

		void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);	

	private:
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();

		// helper functions
		bool isDeviceSuitable(VkPhysicalDevice device);
		std::vector<const char*> getRequiredExtensions();
		bool checkValidationLayerSupport();
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void hasGlfwRequiredInstanceExtensions();
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	};
}


