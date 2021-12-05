#pragma once
#include "pch.h"

#include "platform/Vulkan/VulkanContext.h"
#include "platform/Vulkan/VulkanDescriptors.h"
#include "platform/Vulkan/VulkanPipeline.h"
#include "platform/Vulkan/VulkanBuffer.h"
#include "platform/Vulkan/VulkanTexture.h"

namespace cy3d
{
	//temporary
	struct Camera;

	class SceneRenderer
	{
	public:
		template<typename T>
		using ptr = std::unique_ptr<T>;

	private:
		VulkanContext& _context;
		ptr<VulkanPipeline> _pipeline{ nullptr };
		ptr<VulkanDescriptorPool> _descriptorPool{ nullptr };
		ptr<VulkanDescriptorSetLayout> _descriptorLayout{ nullptr };
		ptr<VulkanDescriptorSets> _descriptorSets{ nullptr };
		std::vector<ptr<VulkanBuffer>> _cameraUbos;
		ptr<VulkanTexture> _texture{ nullptr };

		ptr<VulkanBuffer> _vertexBuffer{ nullptr };
		ptr<VulkanBuffer> _indexBuffer{ nullptr };

	public:
		SceneRenderer(VulkanContext& context);
		~SceneRenderer();

		void beginScene(const Camera& camera);
		void endScene();

	private:
		void init();
		void createPipeline();
		void recreate();



		void createTestVertices();
	};
}


