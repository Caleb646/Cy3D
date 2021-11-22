#include "pch.h"

#include "VulkanDevice.h"
#include "VulkanContext.h"

#include <Logi/Logi.h>


namespace cy3d
{

	// local callback functions
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else 
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) 
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	VulkanDevice::VulkanDevice(VulkanContext& context) : cyContext(context) 
	{
		createInstance(); //init vulkan and create an instance of it
		setupDebugMessenger(); //setup validation layers.
		createSurface(); //connection between the window and vulkan
		pickPhysicalDevice(); //picks the gpu that the program will use
		createLogicalDevice(); //describes what features of the physical device will be used.
		createCommandPool();
	}

	VulkanDevice::~VulkanDevice()
	{
		vkDestroyCommandPool(device_, commandPool, nullptr);
		vkDestroyDevice(device_, nullptr);

		if (enableValidationLayers) 
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface_, nullptr);
		vkDestroyInstance(instance, nullptr);
	}

	void VulkanDevice::createInstance() 
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, enableValidationLayers && checkValidationLayerSupport(), "Validation layers requested, but not available");
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Cy3D";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers) 
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else 
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}
		ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS, "Validation layers requested, but not available");
		hasGlfwRequiredInstanceExtensions();
	}

	/**
	 * @brief Creates a vector of available physical devices and picks
	 * a suitable one.
	*/
	void VulkanDevice::pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		//get the device count
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		ASSERT_ERROR(DEFAULT_LOGGABLE, deviceCount != 0, "Failed to find GPUs with Vulkan support.");
		//std::cout << "Device count: " << deviceCount << std::endl;
		std::vector<VkPhysicalDevice> devices(deviceCount);
		//place the available devices in the devices vector.
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (isDeviceSuitable(device))
			{
				physicalDevice = device;
				break;
			}
		}
		ASSERT_ERROR(DEFAULT_LOGGABLE, physicalDevice != VK_NULL_HANDLE, "Physical device cannot be null.");
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		//std::cout << "physical device: " << properties.deviceName << std::endl;
	}

	/**
	 * @brief 
	*/
	void VulkanDevice::createLogicalDevice()
	{
		//get the supported queue family indices for the picked physical device
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies{ indices.graphicsFamily.value(), indices.presentFamily.value() };

		/**
		 * Vulkan lets you assign priorities to queues to influence the scheduling of
		 * command buffer execution using floating point numbers between 0.0 and 1.0. 
		 * This is required even if there is only a single queue
		*/
		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			//describes the number of queues we want for a single queue family
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		// might not really be necessary anymore because device specific validation layers
		// have been deprecated
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}
		/**
		 * The parameters are the physical device to interface with, 
		 * the queue and usage info we just specified, the optional allocation 
		 * callbacks pointer and a pointer to a variable to store the logical device handle in.
		*/
		ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) == VK_SUCCESS, "Failed to create logical device.");

		/**
		 * We can use the vkGetDeviceQueue function to retrieve queue handles for each queue family.
		 * The parameters are the logical device, queue family, queue index and a pointer to the variable 
		 * to store the queue handle in.
		 * 
		 * If the queue families are the same "have the same queueIndex", then we only need to pass its index once. 
		*/
		vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
		vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);
	}

	/**
	 * @brief Command buffers are executed by submitting them on one of the device queues, like the
	 * graphics and presentation queues we retrieved. Each command pool can only allocate command buffers that are submitted on a single type of queue.
	*/
	void VulkanDevice::createCommandPool() 
	{
		QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		/**
		 * There are two possible flags for command pools:

		 * VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
		 * VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
		*/
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) == VK_SUCCESS, "Failed to create command pool.");
	}

	void VulkanDevice::createSurface() 
	{ 
		cyContext.getWindow()->createWindowSurface(instance, &surface_);
		//window.createWindowSurface(instance, &surface_); 
	}

	/**
	 * @brief Returns true is the neccessary queues, extensions, and present modes are supported
	 * @param device is the current physical device being checked for suitablility
	 * @return True if the device is suitable
	*/
	bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	void VulkanDevice::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		// to add general validation messages to console
		//createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		//to remove general validation messages from console
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;  // Optional
	}

	void VulkanDevice::setupDebugMessenger()
	{
		if (!enableValidationLayers) return;
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	bool VulkanDevice::checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> VulkanDevice::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void VulkanDevice::hasGlfwRequiredInstanceExtensions()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		/*std::cout << "available extensions:" << std::endl;
		std::unordered_set<std::string> available;
		for (const auto& extension : extensions)
		{
			std::cout << "\t" << extension.extensionName << std::endl;
			available.insert(extension.extensionName);
		}*/

		/*std::cout << "required extensions:" << std::endl;
		auto requiredExtensions = getRequiredExtensions();
		for (const auto& required : requiredExtensions)
		{
			std::cout << "\t" << required << std::endl;
			if (available.find(required) == available.end())
			{
				throw std::runtime_error("Missing required glfw extension");
			}
		}*/
	}

	bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(
			device,
			nullptr,
			&extensionCount,
			availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	/**
	 * @brief We need to check which queue families are supported by the device and which one of these supports the commands that we want to use. 
	 * For that purpose we'll add a new function findQueueFamilies that looks for all the queue families we need.
	 * @param device 
	 * @return The supported queues along with their corresponding queue family index
	*/
	QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		//get supported queues count
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);


		/*
		* get the queues themselves
		*
		* The VkQueueFamilyProperties struct contains some details about the queue family,
		* including the type of operations that are supported and the number of queues that
		* can be created based on that family. We need to find at least one queue family that supports VK_QUEUE_GRAPHICS_BIT
		*/
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		// i represents the queue family index for the particular device
		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			/**
			 * It's actually possible that the queue families supporting drawing commands
			 * and the ones supporting presentation do not overlap. 
			 * 
			 * Will treat them as if they were separate queues for a uniform approach
			*/
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport) indices.presentFamily = i;
			if (indices.isComplete()) break;
			i++;
		}

		return indices;
	}

	/**
	 * @brief This function takes the specified VkPhysicalDevice and VkSurfaceKHR window 
	 * surface into account when determining the supported capabilities. All of the support 
	 * querying functions have these two as first parameters because they are the core components of the swap chain.
	 *
	 * Three kinds of properties we need to check:
	 * Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images),
	 * Surface formats (pixel format, color space), and Available presentation modes. 
	*/
	SwapChainSupportDetails VulkanDevice::querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
		}
		return details;
	}

	VkFormat VulkanDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}

	/**
	 * @brief Combine the requirements of the buffer and our own application 
	 * requirements to find the right type of memory to use
	 * @param typeFilter is VkMemoryRequirements.memoryTypeBits. Used to specify the bit field of memory types that are suitable. 
	 * @param properties 
	 * @return 
	*/
	uint32_t VulkanDevice::findMemoryType(decltype(VkMemoryRequirements::memoryTypeBits) typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
		{
			/**
			 * The memoryTypes array consists of VkMemoryType structs that specify the 
			 * heap and properties of each type of memory.
			 * 
			 * The properties define special features of the memory, like being able 
			 * to map it so we can write to it from the CPU. 
			 * 
			 * We may have more than one desirable property, so we should check if the 
			 * result of the bitwise AND is not just non-zero, but equal to the desired 
			 * properties bit field. If there is a memory type suitable for the buffer 
			 * that also has all of the properties we need, then we return its index, 
			 * otherwise we throw an exception.
			*/
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		ASSERT_ERROR(DEFAULT_LOGGABLE, false, "Failed to find memory type.");
	}

	void VulkanDevice::createBuffer(BufferCreationAllocationInfo cyBufferInfo, VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* data)
	{ 
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = cyBufferInfo.bufferSize;
		bufferInfo.usage = cyBufferInfo.usage;

		/**
		 * buffers can also be owned by a specific queue family or be 
		 * shared between multiple at the same time. The buffer will only 
		 * be used from the graphics queue, so we can stick to exclusive access.
		*/
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateBuffer(cyContext.getDevice()->device(), &bufferInfo, nullptr, &buffer) == VK_SUCCESS, "Failed to create buffer.");

		/**
		 * The VkMemoryRequirements struct has three fields:

		 * size: The size of the required amount of memory in bytes, may differ from bufferInfo.size.
		 * 
		 * alignment: The offset in bytes where the buffer begins in the allocated region of memory, 
		 * depends on bufferInfo.usage and bufferInfo.flags.
		 * 
		 * memoryTypeBits: Bit field of the memory types that are suitable for the buffer.
		*/
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, cyBufferInfo.properties);
		ASSERT_ERROR(DEFAULT_LOGGABLE, vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) == VK_SUCCESS, "Failed to allocate memory for buffer.");

		/**
		 * 
		 * If memory allocation was successful, then we can now associate this memory 
		 * with the buffer using vkBindBufferMemory:
		 * 
		 * The fourth param is the offset within the region of memory.
		 * Since this memory is allocated specifically for this the vertex buffer, 
		 * the offset is simply 0. If the offset is non-zero, then it is required to 
		 * be divisible by memRequirements.alignment.
		*/
		vkBindBufferMemory(device_, buffer, bufferMemory, 0);

		if (data != nullptr)
		{
			fillBuffer(data, bufferMemory, bufferInfo.size);
		}
	}

	void VulkanDevice::fillBuffer(VkDeviceMemory& bufferMemory, VkDeviceSize bufferSize, const std::vector<OffsetsInfo>& offsets)
	{
		void* dataDestination;

		/**
		 * This function allows us to access a region of the specified memory
		 * resource defined by an offset and size. The offset and size here are 0 and bufferInfo.size.
		 *
		 * It is also possible to specify the special value VK_WHOLE_SIZE to map all of the memory.
		 *
		 * The last parameter specifies the output for the pointer to the mapped memory.
		*/
		vkMapMemory(device(), bufferMemory, 0, VK_WHOLE_SIZE, 0, &dataDestination);

		uint8_t* ptr = static_cast<uint8_t*>(dataDestination);
		for (auto& info : offsets)
		{
			ptr += info.offset;
			memcpy(ptr, info.data, static_cast<std::size_t>(info.bufferSize));
		}

		vkUnmapMemory(device(), bufferMemory);
	}

	void VulkanDevice::fillBuffer(void* dataSource, VkDeviceMemory& bufferMemory, VkDeviceSize bufferSize)
	{
		void* dataDestination;

		/**
		 * This function allows us to access a region of the specified memory 
		 * resource defined by an offset and size. The offset and size here are 0 and bufferInfo.size.
		 * 
		 * It is also possible to specify the special value VK_WHOLE_SIZE to map all of the memory.
		 * 
		 * The last parameter specifies the output for the pointer to the mapped memory.
		*/
		vkMapMemory(device(), bufferMemory, 0, VK_WHOLE_SIZE, 0, &dataDestination);

		/**
		 * The driver may not immediately copy the 
		 * data into the buffer memory, for example because of caching. It is also possible 
		 * that writes to the buffer are not visible in the mapped memory yet. There are 
		 * two ways to deal with that problem:

		 * Use a memory heap that is host coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT.
		 * Ensures that the mapped memory always matches the contents of the allocated memory.
		 * 
		 * Call vkFlushMappedMemoryRanges after writing to the mapped memory, and call 
		 * vkInvalidateMappedMemoryRanges before reading from the mapped memory
		*/
		memcpy(dataDestination, dataSource, static_cast<std::size_t>(bufferSize));
		vkUnmapMemory(device(), bufferMemory);
	}

	VkCommandBuffer VulkanDevice::beginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}

	void VulkanDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue_); //wait for the transfer queue to become idle.

		vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
	}

	void VulkanDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;  // Optional
		copyRegion.dstOffset = 0;  // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}

	void VulkanDevice::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);
		endSingleTimeCommands(commandBuffer);
	}

	void VulkanDevice::createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device_, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		if (vkBindImageMemory(device_, image, imageMemory, 0) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to bind image memory!");
		}
	}
}

