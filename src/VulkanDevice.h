#pragma once

#include "pch.h"

#include "VulkanWindow.h"
#include "Fwd.hpp"

namespace cy3d
{

	struct OffsetsInfo
	{
		void* data;
		VkDeviceSize bufferSize;
		VkDeviceSize offset;
	};

	struct BufferCreationAllocationInfo
	{
		VkDeviceSize bufferSize;
		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags properties;

		/*
		* Possible Usage bits:
		* VK_BUFFER_USAGE_TRANSFER_SRC_BIT -> Buffer can be used as source in a memory transfer operation.
		* VK_BUFFER_USAGE_TRANSFER_DST_BIT -> Buffer can be used as destination in a memory transfer operation.
		* VK_BUFFER_USAGE_VERTEX_BUFFER_BIT -> Buffer is used for vertexs.
		*/
		static BufferCreationAllocationInfo createDefaultVertexIndexSharedBufferInfo(VkDeviceSize bufferSize)
		{
			return BufferCreationAllocationInfo{ bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
		}


		static BufferCreationAllocationInfo createDefaultStagingBufferInfo(VkDeviceSize bufferSize)
		{
			return BufferCreationAllocationInfo{ bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
		}


		/**
		 * @brief Intended to work in tandem with a staging buffer so the first usage flag is set as
		 * VK_BUFFER_USAGE_TRANSFER_DST_BIT.
		*/
		static BufferCreationAllocationInfo createDefaultDeviceOnlyIndexBufferInfo(VkDeviceSize bufferSize)
		{
			return BufferCreationAllocationInfo{ bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
		}

		/**
		 * @brief Intended to work in tandem with a staging buffer so the first usage flag is set as
		 * VK_BUFFER_USAGE_TRANSFER_DST_BIT.
		*/
		static BufferCreationAllocationInfo createDefaultDeviceOnlyVertexBufferInfo(VkDeviceSize bufferSize)
		{
			return BufferCreationAllocationInfo{ bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
		}

		/**
		 * @brief Will create a vertex buffer info that can be written to by the CPU.
		*/
		static BufferCreationAllocationInfo createDefaultVertexBufferInfo(VkDeviceSize bufferSize)
		{
			return createGPUCPUCoherentBufferInfo(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		}

		/**
		 * @brief Specify a buffer setup that can be accessed by the CPU and GPU.
		*/
		static BufferCreationAllocationInfo createGPUCPUCoherentBufferInfo(VkDeviceSize bufferSize, VkBufferUsageFlags usage)
		{
			return BufferCreationAllocationInfo{ bufferSize, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
		}

		/**
		 * @brief Specify a buffer setup that is intended to only be accessible by the GPU.
		*/
		static BufferCreationAllocationInfo createGPUOnlyBufferInfo(VkDeviceSize bufferSize, VkBufferUsageFlags usage)
		{
			return BufferCreationAllocationInfo{ bufferSize, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
		}
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

#define CY_VK_API_VERSION VK_API_VERSION_1_2

	private:
		VkInstance _instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		/**
		 * The physical graphics card that will be used.
		 * will be destroyed when VkInstance is destroyed.
		*/
		VkPhysicalDevice _physicalDevice{ VK_NULL_HANDLE };
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
		VkDevice _device;

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
		VkSurfaceKHR _surface;

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
		VulkanDevice() = delete;
		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		VulkanDevice& operator=(VulkanDevice&&) = delete;

		VkCommandPool getCommandPool() { return commandPool; }
		VkDevice device() { return _device; }
		VkPhysicalDevice physicalDevice() { return _physicalDevice; }
		VkSurfaceKHR surface() { return _surface; }
		VkInstance instance() { return _instance; }
		VkQueue graphicsQueue() { return graphicsQueue_; }
		VkQueue presentQueue() { return presentQueue_; }

		SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(_physicalDevice); }
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(_physicalDevice); }
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		// Buffer Helper Functions
		void createBuffer(BufferCreationAllocationInfo cyBufferInfo, VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* data = nullptr);
		void fillBuffer(VkDeviceMemory& bufferMemory, VkDeviceSize bufferSize, const std::vector<OffsetsInfo>& offsets);
		void fillBuffer(void* dataToCopy, VkDeviceMemory& bufferMemory, VkDeviceSize bufferSize);
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

		void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

		/**
		 * @brief Casting operator. When VulkanDevice is implicitly or explicity cast to VkDevice
		 * this operator will handle it. No need to call device() to get VkDevice handle.
		*/
		operator VkDevice() { return _device; }

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


