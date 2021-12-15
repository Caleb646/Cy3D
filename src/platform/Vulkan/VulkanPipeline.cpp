#include "pch.h"

#include "VulkanPipeline.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"

#include <Logi/Logi.h>

namespace cy3d
{
    VulkanPipeline::VulkanPipeline(VulkanContext& context, const Ref<VulkanShader>& shader, const PipelineSpec& spec) : _context(context)
    {
        init(shader, spec);
    }

    VulkanPipeline::~VulkanPipeline()
    {
        for (auto& shaderStage : _shaderStages)
        {
            vkDestroyShaderModule(_context.getDevice()->device(), shaderStage.module, nullptr);
        }

        for (auto& descriptorLayout : _descriptorSetLayouts)
        {
            vkDestroyDescriptorSetLayout(_context.getDevice()->device(), descriptorLayout, nullptr);
        }

        cleanup();
    }

    void VulkanPipeline::init(const Ref<VulkanShader>& shader, const PipelineSpec& spec)
    {
        //take ownership of descriptor set layouts
        _descriptorSetLayouts = shader->getDescriptorSetLayouts();
        //take ownership of shader modules
        _shaderStages = shader->getPipelineCreateInfo();
        createLayout();
        createGraphicsPipeline(spec);
    }

    void VulkanPipeline::cleanup()
    {
        if (graphicsPipeline != nullptr)
        {
            vkDestroyPipeline(_context.getDevice()->device(), graphicsPipeline, nullptr);
        }

        if (_pipelineLayout != nullptr)
        {
            vkDestroyPipelineLayout(_context.getDevice()->device(), _pipelineLayout, nullptr);
        }
    }

    void VulkanPipeline::createLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = _descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = _descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0; //used to send data to shaders
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        VK_CHECK(vkCreatePipelineLayout(_context.getDevice()->device(), &pipelineLayoutInfo, nullptr, &_pipelineLayout));
    }

    void VulkanPipeline::createGraphicsPipeline(const PipelineSpec& spec)
    {
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        
        PipelineConfigInfo configInfo{};
        VulkanPipeline::defaultPipelineConfigInfo(spec, configInfo);

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = _shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineInfo.pViewportState = &configInfo.viewportInfo;
        pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
        pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
        pipelineInfo.pDynamicState = nullptr;  // Optional
        pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;

        pipelineInfo.layout = _pipelineLayout;
        pipelineInfo.renderPass = spec.renderpass;
        pipelineInfo.subpass = configInfo.subpass;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
        pipelineInfo.basePipelineIndex = -1;               // Optional


        VK_CHECK(vkCreateGraphicsPipelines(_context.getDevice()->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline));
    }

    bool VulkanPipeline::recreate(const PipelineSpec& spec)
    {
        cleanup(); //destroy graphics pipeline and layout
        createLayout(); //recreate the layout
        createGraphicsPipeline(spec); //recreate the graphics pipeline
        return true;
    }

    void VulkanPipeline::bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }

    /*
    * PUBLIC STATIC METHODS
    */
    
    void VulkanPipeline::defaultPipelineConfigInfo(const PipelineSpec& spec, PipelineConfigInfo& outConfig)
    {
        outConfig.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        outConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        outConfig.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        //describes the relationship between our pipelines output and the target image
        //how the gl_Position values in the vert shader are transformed into the output image
        outConfig.viewport.x = 0.0f;
        outConfig.viewport.y = 0.0f;
        outConfig.viewport.width = static_cast<float>(spec.width);
        outConfig.viewport.height = static_cast<float>(spec.height);

        /**
         * The minDepth and maxDepth values specify the range of depth values to use for the framebuffer. These values
         * must be within the [0.0f, 1.0f] range, but minDepth may be higher than maxDepth.
        */
        outConfig.viewport.minDepth = 0.0f;
        outConfig.viewport.maxDepth = 1.0f;

        //any pixels outside of the scissor will be removed
        outConfig.scissor.offset = { 0, 0 };
        outConfig.scissor.extent = { spec.width, spec.height };

        /**
         * Now this viewport and scissor rectangle need to be combined into a viewport state using the VkPipelineViewportStateCreateInfo struct.
        */
        outConfig.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        outConfig.viewportInfo.viewportCount = 1;
        outConfig.viewportInfo.pViewports = &outConfig.viewport;
        outConfig.viewportInfo.scissorCount = 1;
        outConfig.viewportInfo.pScissors = &outConfig.scissor;


        /**
         * The rasterizer takes the geometry that is shaped by the vertices from the vertex shader and turns it into
         * fragments to be colored by the fragment shader. It also performs depth testing, face culling and the scissor test,
         * and it can be configured to output fragments that fill entire polygons or just the edges (wireframe rendering).
         * All this is configured using the VkPipelineRasterizationStateCreateInfo structure.
        */
        outConfig.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

        /**
         * If depthClampEnable is set to VK_TRUE, then fragments that are beyond the near and far planes are clamped to them as opposed
         * to discarding them. This is useful in some special cases like shadow maps. Using this requires enabling a GPU feature.
        */
        outConfig.rasterizationInfo.depthClampEnable = VK_FALSE; //forces gl_Positions zed position between 0 and 1

        /*
        * If rasterizerDiscardEnable is set to VK_TRUE, then geometry never passes through the rasterizer stage. This basically disables any output to the framebuffer.
        */
        outConfig.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        outConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        outConfig.rasterizationInfo.lineWidth = 1.0f;
        outConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT; //VK_CULL_MODE_NONE;
        outConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //VK_FRONT_FACE_CLOCKWISE;
        outConfig.rasterizationInfo.depthBiasEnable = VK_FALSE;
        outConfig.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
        outConfig.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
        outConfig.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

        /**
         * The VkPipelineMultisampleStateCreateInfo struct configures multisampling, which is one of the ways to perform anti-aliasing. It works
         * by combining the fragment shader results of multiple polygons that rasterize to the same pixel. This mainly occurs along edges, which is also
         * where the most noticeable aliasing artifacts occur. Because it doesn't need to run the fragment shader multiple times if only one polygon maps to a
         * pixel, it is significantly less expensive than simply rendering to a higher resolution and then downscaling. Enabling it requires enabling a GPU feature.
        */
        outConfig.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        outConfig.multisampleInfo.sampleShadingEnable = VK_FALSE;
        outConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        outConfig.multisampleInfo.minSampleShading = 1.0f;           // Optional
        outConfig.multisampleInfo.pSampleMask = nullptr;             // Optional
        outConfig.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
        outConfig.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

        /**
         * After a fragment shader has returned a color, it needs to be combined with the color that is already in the framebuffer. This transformation is known as color blending and there are two ways to do it:
         * Mix the old and new value to produce a final color
         * Combine the old and new value using a bitwise operation
         * There are two types of structs to configure color blending. The first struct, VkPipelineColorBlendAttachmentState contains the configuration per attached framebuffer and the
         * second struct, VkPipelineColorBlendStateCreateInfo contains the global color blending settings
         *
         *
         * If blendEnable is set to VK_FALSE, then the new color from the fragment shader is passed through unmodified. Otherwise,
         * the two mixing operations are performed to compute a new color. The resulting color is AND'd with the colorWriteMask to determine which channels are actually passed through.

         * The most common way to use color blending is to implement alpha blending, where we want the new color to be blended with the old color based on its opacity.
        */
        outConfig.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        outConfig.colorBlendAttachment.blendEnable = VK_FALSE;
        outConfig.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        outConfig.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        outConfig.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
        outConfig.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        outConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        outConfig.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

        outConfig.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        outConfig.colorBlendInfo.logicOpEnable = VK_FALSE;
        outConfig.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
        outConfig.colorBlendInfo.attachmentCount = 1;
        outConfig.colorBlendInfo.pAttachments = &outConfig.colorBlendAttachment;
        outConfig.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
        outConfig.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
        outConfig.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
        outConfig.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

        outConfig.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        outConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
        outConfig.depthStencilInfo.depthWriteEnable = VK_TRUE;
        outConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        outConfig.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        outConfig.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
        outConfig.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
        outConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;
        outConfig.depthStencilInfo.front = {};  // Optional
        outConfig.depthStencilInfo.back = {};   // Optional
    }
}