#pragma once
#include "pch.h"

#include <Logi/Logi.h>

#include "VulkanBuffer.h"
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
		using binding_type = uint32_t;
		using layout_binding_type = VkDescriptorSetLayoutBinding;
		using layout_bindings_type = std::unordered_map<binding_type, layout_binding_type>;

	private:
		VkDescriptorSetLayout _layout{};
		layout_bindings_type _bindings;

		VulkanContext& cyContext;

	public:
		VulkanDescriptorSetLayout(VulkanContext&);
		~VulkanDescriptorSetLayout();

		VulkanDescriptorSetLayout& addBinding(binding_type binding, VkDescriptorType descType, VkShaderStageFlags sFlags, uint32_t descCount = 1);
		VulkanDescriptorSetLayout& build();

		const VkDescriptorSetLayout& getLayout() const { return _layout; }
		const layout_binding_type& getLayoutBinding(const binding_type binding) const
		{
			ASSERT_ERROR(DEFAULT_LOGGABLE, _bindings.count(binding) == 1, "Binding is not valid.");
			return _bindings.at(binding);
		}
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
		VkDescriptorPool _pool;

		VulkanContext& cyContext;

	public:
		VulkanDescriptorPool(VulkanContext&, const std::vector<pool_size_type>&);
		~VulkanDescriptorPool();

		const VkDescriptorPool& getPool() const { return _pool; }
		void allocateDescriptorSets(const VkDescriptorSetLayout* layouts, VkDescriptorSet* sets, uint32_t count) const;
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
		using iterator_type = sets_type::iterator;

	private:
		const VulkanDescriptorPool* _pool;
		const VulkanDescriptorSetLayout* _layout;
		VulkanContext& cyContext;
		uint32_t _count;
		sets_type _sets;

	public:
		VulkanDescriptorSets(VulkanContext& context, const VulkanDescriptorPool* pool, const VulkanDescriptorSetLayout* layout, uint32_t count);

		void writeToBuffer(const VkDescriptorBufferInfo& bufferInfo, std::size_t index, VulkanDescriptorSetLayout::binding_type bindingIndex);

		value_type& at(std::size_t i)
		{
			ASSERT_ERROR(DEFAULT_LOGGABLE, i < _sets.size(), "Index is out of bounds.");
			return _sets.at(i);
		}


		value_type& operator[](std::size_t i)
		{
			ASSERT_ERROR(DEFAULT_LOGGABLE, i < _sets.size(), "Index is out of bounds.");
			return _sets[i];
		}

		iterator_type begin() { return _sets.begin(); }
		iterator_type end() { return _sets.end(); }

	};
}


