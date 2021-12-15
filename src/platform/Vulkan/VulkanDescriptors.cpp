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
    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanContext& context) : cyContext(context)
    {

    }

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanContext& context, const VulkanShader& shader) : cyContext(context)
    {
        init(shader);
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(cyContext.getDevice()->device(), _layout, nullptr);
    }

    VulkanDescriptorSetLayout& VulkanDescriptorSetLayout::addBinding(uint32_t binding, VkDescriptorType descType, VkShaderStageFlags sFlags, uint32_t descCount)
    {
        CY_ASSERT(_bindings.count(binding) == 0);

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
        std::vector<VkDescriptorSetLayoutBinding> layouts;
        for (auto& indexLayout : _bindings) layouts.push_back(indexLayout.second);

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layouts.size());
        layoutInfo.pBindings = layouts.data();
        VK_CHECK(vkCreateDescriptorSetLayout(cyContext.getDevice()->device(), &layoutInfo, nullptr, &_layout));

        return *this;
    }

    void VulkanDescriptorSetLayout::init(const VulkanShader& shader)
    {
        const auto& dInfo = shader.getDescriptorSetsInfo();

        for (const auto [dSetNumber, dMap] : dInfo)
        {
            for (const auto [uboName, uboInfo] : dMap.ubosInfo)
            {
                VkDescriptorSetLayoutBinding bindingInfo{};
                bindingInfo.binding = uboInfo.binding;
                bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                bindingInfo.descriptorCount = dMap.ubosInfo.size();
                bindingInfo.stageFlags = uboInfo.stage;
                bindingInfo.pImmutableSamplers = nullptr; // Optional
                CY_ASSERT(_bindings.count(uboInfo.binding) == 0);
                _bindings[uboInfo.binding] = bindingInfo;
            }

            for (const auto [samplerName, samplerInfo] : dMap.imageSamplersInfo)
            {
                VkDescriptorSetLayoutBinding bindingInfo{};
                bindingInfo.binding = samplerInfo.binding;
                bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                bindingInfo.descriptorCount = dMap.imageSamplersInfo.size();
                bindingInfo.stageFlags = samplerInfo.stage;
                bindingInfo.pImmutableSamplers = nullptr; // Optional
                CY_ASSERT(_bindings.count(samplerInfo.binding) == 0);
                _bindings[samplerInfo.binding] = bindingInfo;
            }
        }
        build();
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

        VK_CHECK(vkCreateDescriptorPool(cyContext.getDevice()->device(), &descriptorPoolInfo, nullptr, &_pool));
    }

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        cleanup();
    }

    void VulkanDescriptorPool::cleanup()
    {
        if (_pool != nullptr)
        {
            vkDestroyDescriptorPool(cyContext.getDevice()->device(), _pool, nullptr);
        }
    }

    void VulkanDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
    {
        vkFreeDescriptorSets(cyContext.getDevice()->device(), _pool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
    }

    void VulkanDescriptorPool::resetDescriptors()
    {
        vkResetDescriptorPool(cyContext.getDevice()->device(), _pool, 0);
    }

    void VulkanDescriptorPool::allocateDescriptorSets(const VkDescriptorSetLayout* layouts, VkDescriptorSet* sets, uint32_t count) const
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _pool;
        allocInfo.descriptorSetCount = count;
        allocInfo.pSetLayouts = layouts;
        VK_CHECK(vkAllocateDescriptorSets(cyContext.getDevice()->device(), &allocInfo, sets));
    }



    /**
   *
   *
   * Vulkan Descriptor Pool Manager
   *
   *
   */
    VulkanDescriptorPoolManager::VulkanDescriptorPoolManager(VulkanContext& context) : _context(context)
    {
        init();
    }

    VulkanDescriptorPoolManager::~VulkanDescriptorPoolManager()
    {
        cleanup();
    }

    void VulkanDescriptorPoolManager::init()
    {
        _currentPool = getPool();
        _usedPools.push_back(_currentPool);
    }

    bool VulkanDescriptorPoolManager::allocateSets(std::vector<VkDescriptorSet>& sets, VkDescriptorSetLayout layout)
    {
        std::vector layouts(sets.size(), layout);

        if (_currentPool == VK_NULL_HANDLE)
        {
            _currentPool = getPool();
            _usedPools.push_back(_currentPool);
        }

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.pSetLayouts = layouts.data();
        allocInfo.descriptorPool = _currentPool;
        allocInfo.descriptorSetCount = sets.size();
        VkResult allocResult = vkAllocateDescriptorSets(_context.getDevice()->device(), &allocInfo, sets.data());

        bool needReallocate = false;
        switch (allocResult)
        {
        case VK_SUCCESS:
            return true;

        case VK_ERROR_FRAGMENTED_POOL:
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            needReallocate = true;
            break;

        default:
        {
            CY_ASSERT(false);//something bad happened
            return false;
        }

        }

        //try to allocate sets again with a new pool
        if (needReallocate)
        {
            _currentPool = getPool();
            _usedPools.push_back(_currentPool);
            VK_CHECK(vkAllocateDescriptorSets(_context.getDevice()->device(), &allocInfo, sets.data()));
        }

        return true;
    }

    bool VulkanDescriptorPoolManager::freeSets(uint32_t poolId, std::vector<VkDescriptorSet>& descriptors)
    {
        CY_ASSERT(false); //not implemented
        return true;
    }

    bool VulkanDescriptorPoolManager::resetPools()
    {
        for (auto& pool : _usedPools)
        {
            vkResetDescriptorPool(_context.getDevice()->device(), pool, 0);
            _openPools.push_back(pool);
        }
        _usedPools.clear();
        _currentPool = VK_NULL_HANDLE;
        return true;
    }

    bool VulkanDescriptorPoolManager::cleanup()
    {
        for (auto pool : _usedPools)
        {
            vkDestroyDescriptorPool(_context.getDevice()->device(), pool, nullptr);
        }

        for (auto pool : _openPools)
        {
            vkDestroyDescriptorPool(_context.getDevice()->device(), pool, nullptr);
        }
        return true;
    }

    VkDescriptorPool VulkanDescriptorPoolManager::createPool()
    {
        std::array<VkDescriptorPoolSize, 2> defaultPoolSizes =
        {
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 20 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20 }
        };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = 0;
        poolInfo.maxSets = 100;
        poolInfo.poolSizeCount = defaultPoolSizes.size();
        poolInfo.pPoolSizes = defaultPoolSizes.data();

        VkDescriptorPool pool;
        vkCreateDescriptorPool(_context.getDevice()->device(), &poolInfo, nullptr, &pool);
        return pool;
    }

    VkDescriptorPool VulkanDescriptorPoolManager::getPool()
    {
        if (_openPools.size() == 0)
        {
            return createPool();
        }

        else if (_openPools.size() > 0)
        {
            auto pool = _openPools.back();
            _openPools.pop_back();
            return pool;
        }
        else
        {
            CY_ASSERT(false); //something weird happened
            return nullptr;
        }
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

    VulkanDescriptorSets::VulkanDescriptorSets(VulkanContext& context, const Ref<VulkanShader>& shader, uint32_t frames) : cyContext(context)
    {
        init(shader, frames);
    }

    VulkanDescriptorSets::VulkanDescriptorSets(VulkanContext& context, const VulkanDescriptorPool* pool, const VulkanDescriptorSetLayout* layout, uint32_t count)
        : cyContext(context), _pool(pool), _layout(layout), _count(count)
    {
        std::vector<VkDescriptorSetLayout> layouts(_count, _layout->getLayout());
        _sets.resize(_count);
        _pool->allocateDescriptorSets(layouts.data(), _sets.data(), _count);
    }

    void VulkanDescriptorSets::init(const Ref<VulkanShader>& shader, uint32_t frames)
    {
        const auto& descLayouts = shader->getDescriptorSetLayouts();

        CY_ASSERT(descLayouts.size() == 1); //cannot handle more than 1 layout until below is fixed
        //create as many sets as there are layouts for each frame
        for (std::size_t frame = 0; frame < frames; frame++)
        {
            _descriptorSets[frame].resize(descLayouts.size());
        }

        for (std::size_t layoutId = 0; layoutId < descLayouts.size(); layoutId++) //frame //set id //sets
        {
            //allocate 
            for (std::size_t frame = 0; frame < frames; frame++)
            {
                //TODO this wont work with multiple layouts
                cyContext.getDescriptorPoolManager()->allocateSets(_descriptorSets[frame], descLayouts.at(layoutId));
            }  
        }
    }

    bool VulkanDescriptorSets::writeBufferToSet(const VkDescriptorBufferInfo& info, std::size_t frame, std::size_t setId, uint32_t bindingIndex)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = _descriptorSets[frame][setId];
        descriptorWrite.dstBinding = bindingIndex;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // temporary
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &info;
        descriptorWrite.pImageInfo = nullptr;
        descriptorWrite.pTexelBufferView = nullptr;
        _writes.push_back(descriptorWrite);
        return true;
    }

    bool VulkanDescriptorSets::writeImageToSet(const VkDescriptorImageInfo& info, std::size_t frame, std::size_t setId, uint32_t bindingIndex)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = _descriptorSets[frame][setId];
        descriptorWrite.dstBinding = bindingIndex;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // temporary
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = nullptr;
        descriptorWrite.pImageInfo = &info;
        descriptorWrite.pTexelBufferView = nullptr;
        _writes.push_back(descriptorWrite);
        return true;
    }

    VulkanDescriptorSets& VulkanDescriptorSets::writeBufferToSet(const VkDescriptorBufferInfo& info, std::size_t index, uint32_t bindingIndex)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = _sets[index];
        descriptorWrite.dstBinding = bindingIndex;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = _layout->getLayoutBinding(bindingIndex).descriptorType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &info;
        descriptorWrite.pImageInfo = nullptr;
        descriptorWrite.pTexelBufferView = nullptr;
        _writes.push_back(descriptorWrite);
        return *this;
    }

    VulkanDescriptorSets& VulkanDescriptorSets::writeImageToSet(const VkDescriptorImageInfo& info, std::size_t index, uint32_t bindingIndex)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = _sets[index];
        descriptorWrite.dstBinding = bindingIndex;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = _layout->getLayoutBinding(bindingIndex).descriptorType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = nullptr;
        descriptorWrite.pImageInfo = &info;
        descriptorWrite.pTexelBufferView = nullptr;
        _writes.push_back(descriptorWrite);
        return *this;
    }

    VulkanDescriptorSets& VulkanDescriptorSets::updateSets()
    {
        vkUpdateDescriptorSets(cyContext.getDevice()->device(), static_cast<uint32_t>(_writes.size()), _writes.data(), 0, nullptr);
        _writes.clear();
        return *this;
    }
}
