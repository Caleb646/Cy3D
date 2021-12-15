#pragma once
#include "pch.h"

#include <Logi/Logi.h>

#include "Vulkan.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "Fwd.hpp"

namespace cy3d
{
	/**
	 *
	 *
	 * Pool Manager
	 *
	 *
	*/
	class VulkanDescriptorPoolManager
	{
	private:
		VulkanContext& _context;
		std::vector<VkDescriptorPool> _usedPools;
		std::vector<VkDescriptorPool> _openPools;
		VkDescriptorPool _currentPool{ VK_NULL_HANDLE };
	public:
		VulkanDescriptorPoolManager(VulkanContext& context);
		~VulkanDescriptorPoolManager();

		bool allocateSets(std::vector<VkDescriptorSet>& sets, VkDescriptorSetLayout layout);
		bool freeSets(uint32_t poolId, std::vector<VkDescriptorSet>& descriptors);
		bool resetPools();

	private:
		void init();
		bool cleanup();
		VkDescriptorPool createPool();
		VkDescriptorPool getPool();
	};


	/**
	 *
	 *
	 * Descriptor Sets
	 *
	 *
	*/
	class VulkanDescriptorSets
	{
	private:
		VulkanContext& _context;
		std::vector<VkWriteDescriptorSet> _writes;

		std::unordered_map<uint32_t, std::vector<VkDescriptorSet>> _descriptorSets; //  frame - set id - descriptor set

	public:
		VulkanDescriptorSets(VulkanContext& context, const Ref<VulkanShader>& shader, uint32_t frames);

		bool writeBufferToSet(const VkDescriptorBufferInfo& info, std::size_t frame,  std::size_t setId, uint32_t bindingIndex);
		bool writeImageToSet(const VkDescriptorImageInfo& info, std::size_t frame, std::size_t setId, uint32_t bindingIndex);
		VulkanDescriptorSets& updateSets();

		std::vector<VkDescriptorSet>& at(std::size_t frame)
		{
			CY_ASSERT(frame < _descriptorSets.size());
			return _descriptorSets.at(frame);
		}

	private:
		void init(const Ref<VulkanShader>& shader, uint32_t frames);

	};
}


