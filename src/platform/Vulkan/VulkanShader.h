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
		VkShaderStageFlagBits stage { VK_SHADER_STAGE_ALL };
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
		std::string _name;
		//uint32_t is the descriptor set the info refers to
		std::unordered_map<uint32_t, ShaderDescriptorSetInfo> _descriptorSetsInfo;
		std::unordered_map<VkShaderStageFlagBits, ShaderData> _source;

	public:
		VulkanShader(const std::string& shaderDirectory);

		std::string getName() { return _name; }
		std::unordered_map<uint32_t, ShaderDescriptorSetInfo>& getDescriptorSetsInfo() { return _descriptorSetsInfo; }
		const std::unordered_map<uint32_t, ShaderDescriptorSetInfo>& getDescriptorSetsInfo() const { return _descriptorSetsInfo; }

	private:
		void init(const std::string& directory);
		void reflect(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& binary);
		bool compile(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outBinary);
		bool readDirectory(const std::string& directory);
		bool readFile(ShaderData& data);
		bool isFileType(const std::filesystem::path& filepath, const std::string& type);

		shaderc_shader_kind vkShaderStageToShaderCStage(VkShaderStageFlagBits stage);
	};
}


