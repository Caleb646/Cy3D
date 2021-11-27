#include "pch.h"

#include "VulkanDescriptors.h"

#include "VulkanContext.h"
#include "VulkanDevice.h"


namespace cy3d
{ 
    /**
     *
     *
     * 
     * Descriptor Layout
     * 
     *
     *
    */
    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanContext& context) : cyContext(context) {}

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(cyContext.getDevice()->device(), _layout, nullptr);
    }

    VulkanDescriptorSetLayout& VulkanDescriptorSetLayout::addBinding(binding_type binding, VkDescriptorType descType, VkShaderStageFlags sFlags, uint32_t descCount)
    {
        ASSERT_ERROR(DEFAULT_LOGGABLE, _bindings.count(binding) == 0, "Binding already exists.");

        VkDescriptorSetLayoutBinding bindingInfo{};
        bindingInfo.binding = binding;
        bindingInfo.descriptorType = descType;
        bindingInfo.descriptorCount = descCount;
        bindingInfo.stageFlags = sFlags;
        bindingInfo.pImmutableSamplers = nullptr; // Optional

        _bindings[binding] = bindingInfo;

        return *this;
    }

    VulkanDescriptorSetLayout& VulkanDescriptorSetLayout::build()
    {
        std::vector<layout_binding_type> layouts;
        for (auto& indexLayout : _bindings) layouts.push_back(indexLayout.second);

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layouts.size());
        layoutInfo.pBindings = layouts.data();
        ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateDescriptorSetLayout(cyContext.getDevice()->device(), &layoutInfo, nullptr, &_layout) == VK_SUCCESS, "Failed to create descriptor set layout.");

        return *this;
    }

    /**
     *
     * 
     *
     * Descriptor Pool
     * 
     *
     *
    */
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

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        vkDestroyDescriptorPool(cyContext.getDevice()->device(), _pool, nullptr);
    }

    void VulkanDescriptorPool::allocateDescriptorSets(const VkDescriptorSetLayout* layouts, VkDescriptorSet* sets, uint32_t count) const
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _pool;
        allocInfo.descriptorSetCount = count;
        allocInfo.pSetLayouts = layouts;
        ASSERT_ERROR(DEFAULT_LOGGABLE, vkAllocateDescriptorSets(cyContext.getDevice()->device(), &allocInfo, sets) == VK_SUCCESS, "Failed to allocate descriptor sets.");
    }


    /**
      *
      *
      *
      * Descriptor Sets
      *
      *
      *
     */
    VulkanDescriptorSets::VulkanDescriptorSets(VulkanContext& context, const VulkanDescriptorPool* pool, const VulkanDescriptorSetLayout* layout, uint32_t count)
        : cyContext(context), _pool(pool), _layout(layout), _count(count)
    {
        std::vector<VkDescriptorSetLayout> layouts(_count, _layout->getLayout());
        _sets.resize(_count);
        _pool->allocateDescriptorSets(layouts.data(), _sets.data(), _count);
    }

    void VulkanDescriptorSets::writeToBuffer(const VkDescriptorBufferInfo& bufferInfo, std::size_t index, VulkanDescriptorSetLayout::binding_type bindingIndex)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = _sets[index];
        descriptorWrite.dstBinding = bindingIndex;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = _layout->getLayoutBinding(bindingIndex).descriptorType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(cyContext.getDevice()->device(), 1, &descriptorWrite, 0, nullptr);
    }
}
