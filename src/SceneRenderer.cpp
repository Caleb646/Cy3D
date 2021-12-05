#include "pch.h"
#include "SceneRenderer.h"
#include "platform/Vulkan/VulkanSwapChain.h"
#include "platform/Vulkan/VulkanRenderer.h"

namespace cy3d
{
	SceneRenderer::SceneRenderer(VulkanContext& context) : _context(context)
	{
		init();
	}

	SceneRenderer::~SceneRenderer()
	{
	}

	void SceneRenderer::beginScene(const Camera& camera)
	{
		_context.getVulkanRenderer()->beginFrame();
		if (_context.getVulkanRenderer()->needsResize()) recreate();

	}

	void SceneRenderer::endScene()
	{
		_context.getVulkanRenderer()->endFrame();
		if (_context.getVulkanRenderer()->needsResize()) recreate();
	}

	void SceneRenderer::init()
	{

		uint32_t numImages = static_cast<uint32_t>(_context.getSwapChain()->imageCount());
		_descriptorPool.reset(new VulkanDescriptorPool(_context,
			{
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numImages },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numImages }
			}
		));

		_descriptorLayout.reset(new VulkanDescriptorSetLayout(_context));
		_descriptorLayout->
			 addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		_texture.reset(new VulkanTexture(_context, "src/resources/textures/viking_room.png"));
		_descriptorSets.reset(new VulkanDescriptorSets(_context, _descriptorPool.get(), _descriptorLayout.get(), numImages));

		for (std::size_t i = 0; i < numImages; i++)
		{
			_descriptorSets->writeBufferToSet(_cameraUbos[i]->descriptorBufferInfo(), i, 0);
			_descriptorSets->writeImageToSet(_texture->descriptorImageInfo(), i, 1);
		}
		_descriptorSets->updateSets();

		createPipeline();
	}

	void SceneRenderer::createPipeline()
	{
		//set up pipeline
		PipelineConfigInfo pipelineConfig{};
		VulkanPipeline::defaultPipelineConfigInfo(pipelineConfig, _context.getWindowWidth(), _context.getWindowHeight());
		pipelineConfig.renderPass = _context.getSwapChain()->getRenderPass();
		PipelineLayoutConfigInfo layoutInfo(&_descriptorLayout->getLayout());
		_pipeline.reset(new VulkanPipeline(_context, pipelineConfig, layoutInfo));
	}

	void SceneRenderer::recreate()
	{
		_pipeline.reset();
		createPipeline();
		_context.getVulkanRenderer()->resetNeedsResize();
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

		BufferCreationAllocationInfo vertexInfo = BufferCreationAllocationInfo::createGPUOnlyBufferInfo(vSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		_vertexBuffer.reset(new VulkanBuffer(_context, vertexInfo, vertices.data()));

	    BufferCreationAllocationInfo indexInfo = BufferCreationAllocationInfo::createGPUOnlyBufferInfo(iSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		_indexBuffer.reset(new VulkanBuffer(_context, indexInfo, indices.data()));

	}
}
