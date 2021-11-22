#include "pch.h"

#include "VulkanAllocator.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"


namespace cy3d
{
	VulkanAllocator::VulkanAllocator(VulkanContext& context) : cyContext(context)
	{
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.vulkanApiVersion = CY_VK_API_VERSION;
		allocatorInfo.physicalDevice = cyContext.getDevice()->physicalDevice();
		allocatorInfo.device = cyContext.getDevice()->device();
		allocatorInfo.instance = cyContext.getDevice()->instance();
		vmaCreateAllocator(&allocatorInfo, &_allocator);
	}
}