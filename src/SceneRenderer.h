#pragma once
#include "pch.h"

#include "platform/Vulkan/VulkanContext.h"
#include "platform/Vulkan/VulkanDescriptors.h"
#include "platform/Vulkan/VulkanPipeline.h"
#include "platform/Vulkan/VulkanBuffer.h"
#include "platform/Vulkan/VulkanTexture.h"

#include "core/core.h"
#include "ShaderManager.h"
#include "Camera.h"

namespace cy3d
{
	struct Mesh {};

	struct CameraUboData
	{
		alignas(16) m3d::mat4f model{};
		alignas(16) m3d::mat4f view{};
		alignas(16) m3d::mat4f proj{};
		
		void update(Camera* camera, float width, float height)
		{
			static std::size_t frames = 0;
			static auto startTime = std::chrono::high_resolution_clock::now();
			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			model = m3d::translate(m3d::mat4f(), m3d::vec3f{ 0.0f, 1.0f, 0.0f });
			view = m3d::lookAt(camera->pos, camera->pos + camera->lookDir, camera->cUp);
			proj = m3d::perspective(m3d::toRadians(90.0f), width / height, 0.1f, 100.0f);
			proj[1][1] *= -1;
		}
	};

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
		
		std::vector<Mesh> _meshes;
		bool _isSceneStart{ false };

	public:
		SceneRenderer(VulkanContext& context);
		~SceneRenderer();

		void beginScene(std::shared_ptr<Camera> camera);
		void endScene();

		bool isSceneStart() { return _isSceneStart; }

	private:
		void init();
		void createPipeline();
		void recreate();
		void flush();

		void basicRenderPass();



		void createTestVertices();
		void testUpdateUbos();
		void testDraw();
	};
}


