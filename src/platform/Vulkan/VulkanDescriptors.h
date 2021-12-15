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
	 *
	 * Descriptor Layouts
	 *
	 *
	 *
	*/
	class VulkanDescriptorSetLayout
	{
	public:
		using layout_bindings_type = std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>;

	private:
		VkDescriptorSetLayout _layout{};
		layout_bindings_type _bindings;

		VulkanContext& cyContext;

	public:
		VulkanDescriptorSetLayout(VulkanContext&);
		VulkanDescriptorSetLayout(VulkanContext&, const VulkanShader& shader);
		~VulkanDescriptorSetLayout();

		VulkanDescriptorSetLayout& addBinding(uint32_t binding, VkDescriptorType descType, VkShaderStageFlags sFlags, uint32_t descCount = 1);
		VulkanDescriptorSetLayout& build();

		VkDescriptorSetLayout& getLayout() { return _layout; }
		const VkDescriptorSetLayout& getLayout() const { return _layout; }	
		const VkDescriptorSetLayoutBinding& getLayoutBinding(const uint32_t binding) const
		{
			CY_ASSERT(_bindings.count(binding) == 1);
			return _bindings.at(binding);
		}

	private:
		void init(const VulkanShader& shader);
	};

	/**
	 *
	 *
	 *
	 * Descriptor Pool
	 *
	 *
	 *
	*/
	class VulkanDescriptorPool
	{
	public:
		using pool_size_type = VkDescriptorPoolSize;

	private:
		uint32_t _maxSets{ 10 };
		VkDescriptorPoolCreateFlags _flags{};
		VkDescriptorPool _pool{ nullptr };

		VulkanContext& cyContext;

	public:
		VulkanDescriptorPool(VulkanContext&, const std::vector<pool_size_type>&);
		~VulkanDescriptorPool();

		void cleanup();
		void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;
		void resetDescriptors();

		const VkDescriptorPool& getPool() const { return _pool; }
		void allocateDescriptorSets(const VkDescriptorSetLayout* layouts, VkDescriptorSet* sets, uint32_t count) const;
	};

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
	 *
	 * Descriptor Sets
	 *
	 *
	 *
	*/
	class VulkanDescriptorSets
	{
	public:
		using value_type = VkDescriptorSet;
		using sets_type = std::vector<value_type>;
		using writes_type = std::vector<VkWriteDescriptorSet>;
		using iterator_type = sets_type::iterator;

	private:
		const VulkanDescriptorPool* _pool;
		const VulkanDescriptorSetLayout* _layout;
		VulkanContext& cyContext;
		uint32_t _count;
		sets_type _sets;
		writes_type _writes;

		std::unordered_map<uint32_t, std::vector<VkDescriptorSet>> _descriptorSets; //  frame - set id - descriptor set

	public:
		VulkanDescriptorSets(VulkanContext& context, const Ref<VulkanShader>& shader, uint32_t frames);
		VulkanDescriptorSets(VulkanContext& context, const VulkanDescriptorPool* pool, const VulkanDescriptorSetLayout* layout, uint32_t count);

		bool writeBufferToSet(const VkDescriptorBufferInfo& info, std::size_t frame,  std::size_t setId, uint32_t bindingIndex);
		bool writeImageToSet(const VkDescriptorImageInfo& info, std::size_t frame, std::size_t setId, uint32_t bindingIndex);

		VulkanDescriptorSets& writeBufferToSet(const VkDescriptorBufferInfo& info, std::size_t index, uint32_t bindingIndex);
		VulkanDescriptorSets& writeImageToSet(const VkDescriptorImageInfo& info, std::size_t index, uint32_t bindingIndex);
		VulkanDescriptorSets& updateSets();

		std::vector<VkDescriptorSet>& at(std::size_t frame)
		{
			CY_ASSERT(frame < _descriptorSets.size());
			return _descriptorSets.at(frame);
		}

		/*value_type& at(std::size_t i)
		{
			CY_ASSERT(i < _sets.size());
			return _sets.at(i);
		}


		value_type& operator[](std::size_t i)
		{
			CY_ASSERT(i < _sets.size());
			return _sets[i];
		}*/

		iterator_type begin() { return _sets.begin(); }
		iterator_type end() { return _sets.end(); }
	private:
		void init(const Ref<VulkanShader>& shader, uint32_t frames);

	};
}


