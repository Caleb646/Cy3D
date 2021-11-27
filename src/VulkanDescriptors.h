#pragma once
#include "pch.h"

#include <Logi/Logi.h>

#include "VulkanBuffer.h"
#include "Fwd.hpp"

namespace cy3d
{
	class VulkanDescriptorPool
	{
	public:
		using pool_size_type = VkDescriptorPoolSize;

	private:
		uint32_t _maxSets{ 1000 };
		VkDescriptorPoolCreateFlags _flags{};
		VkDescriptorPool _pool;

		VulkanContext& cyContext;

	public:
		VulkanDescriptorPool(VulkanContext&, const std::vector<pool_size_type>&);

		VkDescriptorPool& getPool() { return _pool; }
		void allocateDescriptorSets(VulkanDescriptorSetLayout& layout, VkDescriptorSet& set);
	};

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
		VulkanDescriptorSetLayout(VulkanContext&, const layout_bindings_type&);

		VkDescriptorSetLayout& getLayout() { return _layout; }
		layout_binding_type& getLayoutBinding(binding_type binding)
		{
			ASSERT_ERROR(DEFAULT_LOGGABLE, _bindings.count(binding) == 1, "Binding is not valid.");
			return _bindings[binding];
		}
	};

	class VulkanDescriptorSets
	{
	public:
		using sets_type = std::vector<VkDescriptorSet>;
		using iterator_type = sets_type::iterator;

	private:
		VulkanDescriptorPool& _pool;
		VulkanDescriptorSetLayout& _layout;
		VulkanContext& cyContext;
		uint32_t _count;
		sets_type _sets;

	public:
		VulkanDescriptorSets(VulkanContext& context, VulkanDescriptorPool& pool, VulkanDescriptorSetLayout& layout, uint32_t count);

		void writeToBuffer(VkDescriptorBufferInfo& bufferInfo, std::size_t index, VulkanDescriptorSetLayout::binding_type bindingIndex);

		iterator_type begin() { return _sets.begin(); }
		iterator_type end() { return _sets.end(); }

	};
}


