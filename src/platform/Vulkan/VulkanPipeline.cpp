#include "pch.h"

#include "VulkanPipeline.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"

#include <Logi/Logi.h>

namespace cy3d
{

    constexpr auto DEFAULT_VERT_SHADER_PATH = "src/resources/shaders/SimpleShader.vert.spv";
    constexpr auto DEFAULT_FRAG_SHADER_PATH = "src/resources/shaders/SimpleShader.frag.spv";

    VulkanPipeline::VulkanPipeline(VulkanContext& context, PipelineConfigInfo& config, PipelineLayoutConfigInfo& layoutInfo) : cyContext(context)
    {
        createGraphicsPipeline(config, layoutInfo); 
    }

    VulkanPipeline::~VulkanPipeline()
    {
        cleanup();
    }

    void VulkanPipeline::cleanup()
    {
        if (vertShaderModule != nullptr)
        {
            vkDestroyShaderModule(cyContext.getDevice()->device(), fragShaderModule, nullptr);
        }
        if (fragShaderModule != nullptr)
        {
            vkDestroyShaderModule(cyContext.getDevice()->device(), vertShaderModule, nullptr);
        }
        if (graphicsPipeline != nullptr)
        {
            vkDestroyPipeline(cyContext.getDevice()->device(), graphicsPipeline, nullptr);
        } 
        if (_pipelineLayout != nullptr)
        {
            vkDestroyPipelineLayout(cyContext.getDevice()->device(), _pipelineLayout, nullptr);
        }
    }

    void VulkanPipeline::bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }

    void VulkanPipeline::createGraphicsPipeline(PipelineConfigInfo& configInfo, PipelineLayoutConfigInfo& layoutInfo)
    {


        ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreatePipelineLayout(cyContext.getDevice()->device(), &layoutInfo.pipelineLayoutInfo, nullptr, &_pipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout");

        ASSERT_ERROR(DEFAULT_LOGGABLE, configInfo.renderPass != nullptr, "Did not include a valid renderpass in Pipeline Config Info.");

        auto vertCode = readFile(DEFAULT_VERT_SHADER_PATH);
        auto fragCode = readFile(DEFAULT_FRAG_SHADER_PATH);

        createShaderModule(vertCode, &vertShaderModule);
        createShaderModule(fragCode, &fragShaderModule);

        /**
         * To actually use the shaders we'll need to assign them to a specific pipeline stage through 
         * VkPipelineShaderStageCreateInfo structures as part of the actual pipeline creation process.
        */
        VkPipelineShaderStageCreateInfo shaderStages[2];

        //vertex shader
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT; //tell vulkan this is a vertex shader.
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName = "main"; //name of entry function in vertex shader
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr; // allows you to specify values for shader constants

        //fragment shader
        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;



        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();
        /**
         * The VkPipelineVertexInputStateCreateInfo structure describes the format of the vertex data that will be passed to the vertex shader. 
         * It describes this in roughly two ways:

         * Bindings: spacing between data and whether the data is per-vertex or per-instance (see instancing).
         * 
         * Attribute descriptions: type of the attributes passed to the vertex shader, which binding to load them from and at which offset.
         * 
        */

       /* VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;*/

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineInfo.pViewportState = &configInfo.viewportInfo;
        pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
        pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
        pipelineInfo.pDynamicState = nullptr;  // Optional
        pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;

        pipelineInfo.layout = _pipelineLayout;
        pipelineInfo.renderPass = configInfo.renderPass;
        pipelineInfo.subpass = configInfo.subpass;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
        pipelineInfo.basePipelineIndex = -1;               // Optional


        ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateGraphicsPipelines(cyContext.getDevice()->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) == VK_SUCCESS, "Failed to create graphics pipeline.");
    }

    /**
     * @brief 
     * @param code is a buffer with bytecode
     * @param shaderModule 
    */
    void VulkanPipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        //std::vector ensures that the data satisfies the worst case alignment requirements.
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        
        ASSERT_ERROR(DEFAULT_LOGGABLE, vkCreateShaderModule(cyContext.getDevice()->device(), &createInfo, nullptr, shaderModule) == VK_SUCCESS, "Failed to create shader module.");
    }

    /*
    * PUBLIC STATIC METHODS
    */
    
    /**
     * @brief 
     * @param configInfo 
     * @param width is the width of the swapChainExtent not of the window
     * @param height is the height of the swapChainExtent not of the window
    */
    void VulkanPipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo, uint32_t width, uint32_t height)
    {
        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        //describes the relationship between our pipelines output and the target image
        //how the gl_Position values in the vert shader are transformed into the output image
        configInfo.viewport.x = 0.0f;
        configInfo.viewport.y = 0.0f;
        configInfo.viewport.width = static_cast<float>(width);
        configInfo.viewport.height = static_cast<float>(height);

        /**
         * The minDepth and maxDepth values specify the range of depth values to use for the framebuffer. These values 
         * must be within the [0.0f, 1.0f] range, but minDepth may be higher than maxDepth.
        */
        configInfo.viewport.minDepth = 0.0f;
        configInfo.viewport.maxDepth = 1.0f;

        //any pixels outside of the scissor will be removed
        configInfo.scissor.offset = { 0, 0 };
        configInfo.scissor.extent = { width, height };

        /**
         * Now this viewport and scissor rectangle need to be combined into a viewport state using the VkPipelineViewportStateCreateInfo struct.
        */
        configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.pViewports = &configInfo.viewport;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.pScissors = &configInfo.scissor;


        /**
         * The rasterizer takes the geometry that is shaped by the vertices from the vertex shader and turns it into 
         * fragments to be colored by the fragment shader. It also performs depth testing, face culling and the scissor test, 
         * and it can be configured to output fragments that fill entire polygons or just the edges (wireframe rendering). 
         * All this is configured using the VkPipelineRasterizationStateCreateInfo structure.
        */
        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

        /**
         * If depthClampEnable is set to VK_TRUE, then fragments that are beyond the near and far planes are clamped to them as opposed
         * to discarding them. This is useful in some special cases like shadow maps. Using this requires enabling a GPU feature.
        */
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE; //forces gl_Positions zed position between 0 and 1

        /*
        * If rasterizerDiscardEnable is set to VK_TRUE, then geometry never passes through the rasterizer stage. This basically disables any output to the framebuffer.
        */
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT; //VK_CULL_MODE_NONE;
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //VK_FRONT_FACE_CLOCKWISE;
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
        configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

        /**
         * The VkPipelineMultisampleStateCreateInfo struct configures multisampling, which is one of the ways to perform anti-aliasing. It works 
         * by combining the fragment shader results of multiple polygons that rasterize to the same pixel. This mainly occurs along edges, which is also 
         * where the most noticeable aliasing artifacts occur. Because it doesn't need to run the fragment shader multiple times if only one polygon maps to a 
         * pixel, it is significantly less expensive than simply rendering to a higher resolution and then downscaling. Enabling it requires enabling a GPU feature.
        */
        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
        configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
        configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
        configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

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
        configInfo.colorBlendAttachment.colorWriteMask =  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
        configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.front = {};  // Optional
        configInfo.depthStencilInfo.back = {};   // Optional
    }

    /*
    * PRIVATE STATIC METHODS
    */

    std::vector<char> VulkanPipeline::readFile(const std::string& filename)
    {
        std::ifstream file{ filename, std::ios::ate | std::ios::binary };
        ASSERT_ERROR(DEFAULT_LOGGABLE, file.is_open() == true, "Failed to open file: " + filename);
        std::size_t fileSize = static_cast<std::size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }
}