#include "pch.h"
#include "SceneRenderer.h"
#include "platform/Vulkan/VulkanSwapChain.h"
#include "platform/Vulkan/VulkanRenderer.h"
#include "core/core.h"

namespace cy3d
{
	SceneRenderer::SceneRenderer(VulkanContext& context) : _context(context)
	{
		init();
	}

	SceneRenderer::~SceneRenderer()
	{
	}

	void SceneRenderer::beginScene(std::shared_ptr<Camera> camera)
	{

		CY_ASSERT(isSceneStart() == false);
		_context.getRenderer()->beginFrame();
		if (_context.getRenderer()->needsResize())
		{
			recreate(); 
			return;
		}
		_isSceneStart = true;


		CameraUboData cd{};
		/*cd.translation = m3d::Mat4f::getTranslation(m3d::Vec4f(camera->pos, 1.0f));
		cd.view = m3d::Mat4f::getLookAt(camera->pos, { 0.0f, 0.0f, 0.0f }, camera->cUp);
		cd.proj = camera->projectionMatrix;*/
		cd.update(camera.get(), _context.getWindowWidth(), _context.getWindowHeight());
		_cameraUbos[_context.getRenderer()->getCurrentImageIndex()]->setData(&cd, 0);
		//TESTING ONLY
		//testUpdateUbos();
		//END
	}

	void SceneRenderer::endScene()
	{
		CY_ASSERT(isSceneStart() == true);

		flush();

		_context.getRenderer()->endFrame();
		if (_context.getRenderer()->needsResize()) recreate();

		_isSceneStart = false;
	}

	void SceneRenderer::init()
	{
		uint32_t numImages = static_cast<uint32_t>(_context.getSwapChain()->imageCount());

		_context.getShaderManager()->add("resources/shaders/simpleshaders", "SimpleShader");
		const auto& shader = _context.getShaderManager()->get("SimpleShader");

		BufferCreateInfo cameraInfo = shader->getDescriptorSetUBOInfo(0, "CameraUboData").createInfo;

		_cameraUbos.resize(numImages);
		for (std::size_t i = 0; i < numImages; i++)
		{
			_cameraUbos[i].reset(new VulkanBuffer(_context, cameraInfo));
		}
		_texture.reset(new VulkanTexture(_context, "resources/textures/viking_room.png"));
		_descriptorSets.reset(new VulkanDescriptorSets(_context, shader, numImages));


		for (std::size_t i = 0; i < numImages; i++)
		{
			_descriptorSets->writeBufferToSet(_cameraUbos[i]->descriptorInfo(), i, 0, 0);
			_descriptorSets->writeImageToSet(_texture->descriptorInfo(), i, 0, 1);
		}
		_descriptorSets->updateSets();

		PipelineSpec spec{};
		spec.width = _context.getWindowWidth();
		spec.height = _context.getWindowHeight();
		spec.renderpass = _context.getSwapChain()->getRenderPass();
		_pipeline.reset(new VulkanPipeline(_context, shader, spec));

		//TESTING ONLY
		createTestVertices();
	}

	void SceneRenderer::createPipeline()
	{

	}

	void SceneRenderer::recreate()
	{
		PipelineSpec spec{};
		spec.width = _context.getWindowWidth();
		spec.height = _context.getWindowHeight();
		spec.renderpass = _context.getSwapChain()->getRenderPass();
		_pipeline->recreate(spec);

		_context.getRenderer()->resetNeedsResize();
	}

	void SceneRenderer::flush()
	{
		CY_ASSERT(isSceneStart() == true);
		basicRenderPass();
		_meshes.clear();
	}

	void SceneRenderer::basicRenderPass()
	{
		CY_ASSERT(isSceneStart() == true);
		_context.getRenderer()->beginRenderPass(_context.getSwapChain()->getRenderPass());

		//TESTING ONLY
		testDraw();

		//for (auto& mesh : _meshes)
		//{
		//	//TODO draw meshs here.
		//}

		_context.getRenderer()->endRenderPass();
	}


	void SceneRenderer::createTestVertices()
	{
		std::vector<Vertex> vertices =
		{
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

			{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
		};

		std::vector<uint16_t> indices =
		{
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		};

		VkDeviceSize vSize = sizeof(vertices[0]) * vertices.size();
		VkDeviceSize iSize = sizeof(indices[0]) * indices.size();

		BufferCreateInfo vertexInfo = BufferCreateInfo::createGPUOnlyBufferInfo(vSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		_vertexBuffer.reset(new VulkanBuffer(_context, vertexInfo, vertices.data()));

	    BufferCreateInfo indexInfo = BufferCreateInfo::createGPUOnlyBufferInfo(iSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		_indexBuffer.reset(new VulkanBuffer(_context, indexInfo, indices.data()));

	}

	void SceneRenderer::testUpdateUbos()
	{
		//FOR TESTING ONLY
		UniformBufferObject ubo{};
		ubo.update(_context.getSwapChain()->getWidth(), _context.getSwapChain()->getHeight());
		_cameraUbos[_context.getRenderer()->getCurrentImageIndex()]->setData(&ubo, 0);
		//END
	}


	void SceneRenderer::testDraw()
	{
		//FOR Testing only

		VkBuffer vertexBuffers[] = { _vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };



		/**
			* The second parameter specifies if the pipeline object is a graphics or compute pipeline.
			* We've now told Vulkan which operations to execute in the graphics pipeline and which attachment to use in the fragment shader,
		*/
		_pipeline->bind(_context.getRenderer()->getCurrentCommandBuffer());
		vkCmdBindDescriptorSets(_context.getRenderer()->getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->getPipelineLayout(), 0, 1, _descriptorSets->at(_context.getCurrentFrameIndex()).data(), 0, nullptr);

		/**
			* The vkCmdBindVertexBuffers function is used to bind vertex buffers to bindings, like the
			* one we set up in the previous chapter. The first two parameters, besides the command buffer,
			* specify the offset and number of bindings we're going to specify vertex buffers for. The last
			* two parameters specify the array of vertex buffers to bind and the byte offsets to start reading
			* vertex data from.
		*/
		vkCmdBindVertexBuffers(_context.getRenderer()->getCurrentCommandBuffer(), 0, 1, vertexBuffers, offsets);


		/**
			* Parameters other than the command buffer are the index buffer, a byte offset into it, and the type of index data.
			* The possible types are VK_INDEX_TYPE_UINT16 and VK_INDEX_TYPE_UINT32.
		*/
		//vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer.get()->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);

		//Binding with the vertex and index buffers being combined.
		//vkCmdBindIndexBuffer(_context.getRenderer()->getCurrentCommandBuffer(), _omniBuffer->getBuffer(), _omniBuffer->offset(), VK_INDEX_TYPE_UINT16);

		vkCmdBindIndexBuffer(_context.getRenderer()->getCurrentCommandBuffer(), _indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT16);

		/**
			* vkCmdDraw has the following parameters, aside from the command buffer:
			*
			* vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
			* instanceCount: Used for instanced rendering, use 1 if you're not doing that.
			* firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
			* firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
		*/
		//vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertexBuffer.get()->size()), 1, 0, 0); 

		vkCmdDrawIndexed(_context.getRenderer()->getCurrentCommandBuffer(), static_cast<uint32_t>(_indexBuffer->count()), 1, 0, 0, 0);

		//vkCmdEndRenderPass(commandBuffers[i]);

		//ASSERT_ERROR(DEFAULT_LOGGABLE, vkEndCommandBuffer(commandBuffers[i]) == VK_SUCCESS, "Failed to record command buffer.");


		//END
	}
}
