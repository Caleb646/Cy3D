#include "pch.h"

#include "VulkanDescriptors.h"

#include "VulkanContext.h"
#include "VulkanDevice.h"


namespace cy3d
{
	VulkanDescriptorPool::VulkanDescriptorPool(VulkanContext& context, const std::vector<pool_size_type>& poolSizes) : cyContext(context)
	{
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = _maxSets;
        descriptorPoolInfo.flags = _flags;

        ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateDescriptorPool(cyContext.getDevice()->device(), &descriptorPoolInfo, nullptr, &_pool) == VK_SUCCESS, "Failed to create descriptor pool.");
	}

    void VulkanDescriptorPool::allocateDescriptorSets(VulkanDescriptorSetLayout& layout, VkDescriptorSet& set)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout.getLayout();
        ASSERT_ERROR(DEFAULT_LOGGABLE, vkAllocateDescriptorSets(cyContext.getDevice()->device(), &allocInfo, &set) == VK_SUCCESS, "Failed to allocate descriptor sets.");
    }


    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanContext& context, const layout_bindings_type& bindings) : cyContext(context), _bindings(bindings)
    {
        std::vector<layout_binding_type> layouts;
        for (auto& indexLayout : bindings) layouts.push_back(indexLayout.second);

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layouts.size());
        layoutInfo.pBindings = layouts.data();
        ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateDescriptorSetLayout(cyContext.getDevice()->device(), &layoutInfo, nullptr, &_layout) == VK_SUCCESS, "Failed to create descriptor set layout.");
    }

    VulkanDescriptorSets::VulkanDescriptorSets(VulkanContext& context, VulkanDescriptorPool& pool, VulkanDescriptorSetLayout& layout, uint32_t count) 
        : cyContext(context), _pool(pool), _layout(layout), _count(count)
    {
        std::vector<VkDescriptorSetLayout> layouts(_count, layout.getLayout());

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool.getPool();
        allocInfo.descriptorSetCount = _count;
        allocInfo.pSetLayouts = layouts.data();

        _sets.resize(_count);
        ASSERT_ERROR(DEFAULT_LOGGABLE, vkAllocateDescriptorSets(cyContext.getDevice()->device(), &allocInfo, _sets.data()) == VK_SUCCESS, "Failed to allocate descriptor sets.");
    }

    void VulkanDescriptorSets::writeToBuffer(VkDescriptorBufferInfo& bufferInfo, std::size_t index, VulkanDescriptorSetLayout::binding_type bindingIndex)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = _sets[index];
        descriptorWrite.dstBinding = bindingIndex;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = _layout.getLayoutBinding(bindingIndex).descriptorType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(cyContext.getDevice()->device(), 1, &descriptorWrite, 0, nullptr);
    }
}
