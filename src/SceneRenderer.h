#pragma once
#include "pch.h"

#include "core/core.h"
#include "platform/Vulkan/VulkanContext.h"
#include "platform/Vulkan/VulkanDescriptors.h"
#include "platform/Vulkan/VulkanPipeline.h"
#include "platform/Vulkan/VulkanBuffer.h"
#include "platform/Vulkan/VulkanTexture.h"

#include "src/Camera.h"

namespace cy3d
{
	//temporary
	//struct Camera {};
	struct Mesh {};

	struct CameraUboData
	{
		alignas(16) m3d::mat4f model{};
		alignas(16) m3d::mat4f view{};
		alignas(16) m3d::mat4f proj{};
		//alignas(16) glm::mat4 model{};
		//alignas(16) glm::mat4 view{};
		//alignas(16) glm::mat4 proj{};
		
		void update(Camera* camera, float width, float height)
		{
			static std::size_t frames = 0;
			static auto startTime = std::chrono::high_resolution_clock::now();
			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			
			//model = glm::translate(glm::mat4(1.0f), glm::vec3(camera->pos.x(), camera->pos.y(), camera->pos.z()));
			//view = glm::lookAt(
			//	glm::vec3(camera->pos.x(), camera->pos.y(), camera->pos.z()), 
			//	glm::vec3(camera->pos.x(), camera->pos.y(), camera->pos.z()) + glm::vec3(camera->lookDir.x(), camera->lookDir.y(), camera->lookDir.z()),
			//	glm::vec3(0.0f, 0.0f, 1.0f));

			//proj = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 10.0f);

			//FOR TESTING ONLY
			//model.getRotatedZ(time * m3d::degreesToRadians(90.0f));
			//model.getTranslated(m3d::vec4f(camera->pos, 1.0f));
			//view = m3d::mat4f::getLookAt({ 2.0f, 2.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
			//proj = m3d::mat4f::getPerspective(m3d::degreesToRadians(45.0f), width / height, 0.1f, 10.0f);
			//proj[1][1] *= -1;

			//each model will have its own translation this is just for testing.
			model = m3d::translate(m3d::mat4f(), m3d::vec3f{0.0f, 1.0f, 0.0f});
			//view = m3d::lookAt(camera->pos, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
			//view = m3d::lookAt(camera->pos, m3d::vec3f{1.f, 1.f, 1.f} + camera->lookDir, camera->cUp);
			view = m3d::lookAt(camera->pos, camera->pos + camera->lookDir, camera->cUp);
			proj = m3d::perspective(m3d::toRadians(90.0f), width / height, 0.1f, 100.0f);
			proj[1][1] *= -1;


			/*glm::mat4 gModel(1.0f);
			gModel = glm::translate(glm::mat4(1.0f), glm::vec3(camera->pos.x(), camera->pos.y(), camera->pos.z()));
			glm::mat4 gview(1.0f);
			gview = glm::lookAt
			(
				glm::vec3(camera->pos.x(), camera->pos.y(), camera->pos.z()), 
				glm::vec3(camera->pos.x(), camera->pos.y(), camera->pos.z()) + glm::vec3(camera->lookDir.x(), camera->lookDir.y(), camera->lookDir.z()),
				glm::vec3(camera->cUp.x(), camera->cUp.y(), camera->cUp.z())
			);

			glm::mat4 gproj(1.0f);
			gproj = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 10.0f);
			gproj[1][1] *= -1;*/
			

			//glm::mat4 gModel(1.0f);
			//gModel = glm::translate(glm::mat4(1.0f), glm::vec3(camera->pos.x(), camera->pos.y(), camera->pos.z()));
			//glm::mat4 gview(1.0f);
			//gview = glm::lookAt(
			//	glm::vec3(2.0f, 2.0f, 2.0f),
			//	glm::vec3(0.0f, 0.0f, 0.0f),
			//	glm::vec3(0.0f, 0.0f, 1.0f));

			//glm::mat4 gproj(1.0f);
			//gproj = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 10.0f);
			//gproj[1][1] *= -1;

			//areEqual(model, gModel, "Model");
			//areEqual(view, gview, "View");
			//areEqual(proj, gproj, "Projection");

			//model.getRotatedZ(time * m3d::degreesToRadians(90.0f));
		//    view = m3d::Mat4f::getLookAt({ 2.0f, 2.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f, 1.0f});
		//    proj.getPerspectived(m3d::degreesToRadians(45.0f), width / height, 0.1f, 10.0f);
		//    proj(1, 1) *= -1;

			if (frames % 10000 == 0)
			{
				/*view.print();
				camera->getLookAt().print();
				(camera->pos + camera->lookDir).print();
				frames = 0;*/
				//float* buffer = view;
				//std::byte* buffer = static_cast<std::byte*>(view);
				//glm::mat4 m{};
				//m3d::mat4f mm{};
				//LOG_DEBUG(DEFAULT_LOGGABLE, "" + sizeof(glm::mat4));
				//LOG_DEBUG(DEFAULT_LOGGABLE, "\n" + sizeof(m3d::mat4f));
			}
			//frames++;
			
			//proj = m3d::mat4f::getPerspective(m3d::degreesToRadians(45.0f), width / height, 0.1f, 10.0f);
			//proj = camera->projectionMatrix;//.getPerspectived(m3d::degreesToRadians(45.0f), height / width, 0.1f, 10.0f);
			//proj(1, 1) *= -1;
		}

		/*void areEqual(m3d::mat4f& m, glm::mat4& g, std::string name)
		{
			for (std::size_t i = 0; i < 4; i++)
			{
				for (std::size_t j = 0; j < 4; j++)
				{
					float mv = m[i][j];
					float gv = g[i][j];
					float res = std::abs(mv - gv);
					CY_ASSERT(res < 0.01f);
				}
			}
		}*/
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


