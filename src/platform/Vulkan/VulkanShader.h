#pragma once
#include "pch.h"

#include "../../core/core.h"
#include "VulkanContext.h"
#include "VulkanBufferTypes.h"

namespace cy3d
{
	constexpr auto VERT_EXTENSION = ".vert";
	constexpr auto FRAG_EXTENSION = ".frag";

	struct ShaderData
	{
		std::string source{};
		std::filesystem::path filepath{};
	};

	struct ShaderImageSamplerSetInfo
	{
		uint32_t binding;
		uint32_t descriptorSet;
		VkShaderStageFlagBits stage{ VK_SHADER_STAGE_ALL };
	};

	struct ShaderUBOSetInfo
	{
		uint32_t binding;
		uint32_t descriptorSet;
		BufferCreateInfo createInfo;
		VkShaderStageFlagBits stage{ VK_SHADER_STAGE_ALL };
	};

	struct ShaderDescriptorSetInfo
	{
		//std::string is the name of the sampler or ubo
		std::unordered_map<std::string, ShaderUBOSetInfo> ubosInfo;
		std::unordered_map<std::string, ShaderImageSamplerSetInfo> imageSamplersInfo;
	};

	class VulkanShader
	{
		//VK_SHADER_STAGE_VERTEX_BIT,
		//VK_SHADER_STAGE_FRAGMENT_BIT
	private:
		VulkanContext& _context;
		std::string _name;
		//uint32_t is the descriptor set id the info refers to
		std::unordered_map<uint32_t, ShaderDescriptorSetInfo> _descriptorSetsInfo;
		std::unordered_map<VkShaderStageFlagBits, ShaderData> _source;
		std::vector<VkPipelineShaderStageCreateInfo> _pipelineCreateInfo;
		//index is the setId
		std::vector<VkDescriptorSetLayout> _descriptorSetLayouts;
		//std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkDescriptorSet>> _descriptorSets;

	public:
		VulkanShader(VulkanContext& context, const std::string& shaderDirectory);

		std::string getName() { return _name; }

		ShaderUBOSetInfo getDescriptorSetUBOInfo(uint32_t setId, const std::string& name)
		{ 
			CY_ASSERT(_descriptorSetsInfo.count(setId) != 0);
			CY_ASSERT(_descriptorSetsInfo[setId].ubosInfo.count(name) != 0);
			return _descriptorSetsInfo.at(setId).ubosInfo.at(name);
		}
		const ShaderUBOSetInfo& getDescriptorSetUBOInfo(uint32_t setId, const std::string& name) const
		{ 
			CY_ASSERT(_descriptorSetsInfo.count(setId) != 0);
			CY_ASSERT(_descriptorSetsInfo.at(setId).ubosInfo.count(name) != 0);
			return _descriptorSetsInfo.at(setId).ubosInfo.at(name);
		}

		std::unordered_map<uint32_t, ShaderDescriptorSetInfo>& getDescriptorSetsInfo() { return _descriptorSetsInfo; }
		const std::unordered_map<uint32_t, ShaderDescriptorSetInfo>& getDescriptorSetsInfo() const { return _descriptorSetsInfo; }

		std::vector<VkPipelineShaderStageCreateInfo>& getPipelineCreateInfo() { return _pipelineCreateInfo; }
		const std::vector<VkPipelineShaderStageCreateInfo>& getPipelineCreateInfo() const { return _pipelineCreateInfo; }

		std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts() { return _descriptorSetLayouts; }
		const std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts() const { return _descriptorSetLayouts; }

	private:
		void init(const std::string& directory);
		bool createDescriptorSetLayouts();
		bool createShaderModules(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& binary);
		void reflect(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& binary);
		bool compile(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outBinary);
		bool readDirectory(const std::string& directory);
		bool readFile(ShaderData& data);
		bool isFileType(const std::filesystem::path& filepath, const std::string& type);
		bool stripFilenameExtension();
		shaderc_shader_kind vkShaderStageToShaderCStage(VkShaderStageFlagBits stage);
	};
}


