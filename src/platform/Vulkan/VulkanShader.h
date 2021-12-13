#pragma once
#include "pch.h"

#include "../../core/CoreLogging.h"
#include "VulkanContext.h"

namespace cy3d
{
	constexpr auto VERT_EXTENSION = ".vert";
	constexpr auto FRAG_EXTENSION = ".frag";

	struct ShaderData
	{
		std::string source{};
		std::filesystem::path filepath{};
	};

	class VulkanShader
	{
		//VK_SHADER_STAGE_VERTEX_BIT,
		//VK_SHADER_STAGE_FRAGMENT_BIT

	private:
		VulkanContext& _context;
		std::string _name;
		std::unordered_map<VkShaderStageFlagBits, ShaderData> _source;

	public:
		VulkanShader(VulkanContext& context, const std::string& shaderDirectory);

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


