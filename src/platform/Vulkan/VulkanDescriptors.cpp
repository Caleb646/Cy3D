#include "pch.h"

#include "VulkanDescriptors.h"

#include "VulkanContext.h"
#include "VulkanDevice.h"


namespace cy3d
{ 

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
    VulkanDescriptorSets::VulkanDescriptorSets(VulkanContext& context, const Ref<VulkanShader>& shader, uint32_t frames) : _context(context)
    {
        init(shader, frames);
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
                _context.getDescriptorPoolManager()->allocateSets(_descriptorSets[frame], descLayouts.at(layoutId));
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

    VulkanDescriptorSets& VulkanDescriptorSets::updateSets()
    {
        vkUpdateDescriptorSets(_context.getDevice()->device(), static_cast<uint32_t>(_writes.size()), _writes.data(), 0, nullptr);
        _writes.clear();
        return *this;
    }
}
