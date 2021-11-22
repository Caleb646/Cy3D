#pragma once
#include "pch.h"

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#include "Fwd.hpp"

namespace cy3d
{
	class VulkanAllocator
	{
	private:
		VulkanContext& cyContext;
		VmaAllocator _allocator{};


	public:
		VulkanAllocator(VulkanContext&);

		VulkanAllocator() = delete;
		VulkanAllocator(const VulkanAllocator&) = delete;
		VulkanAllocator(VulkanAllocator&&) = delete;
		VulkanAllocator& operator=(const VulkanAllocator&) = delete;
	};
}


