#pragma once
#include "pch.h"

#include "VulkanContext.h"

namespace cy3d
{
	constexpr auto VERT_EXTENSION = ".vert";
	constexpr auto FRAG_EXTENSION = ".frag";

	class VulkanShader
	{
		//VK_SHADER_STAGE_VERTEX_BIT,
		//VK_SHADER_STAGE_FRAGMENT_BIT

	private:
		VulkanContext& _context;
		std::unordered_map<VkShaderStageFlagBits, std::vector<std::string>> _shaderSource;

	public:
		VulkanShader(VulkanContext& context, const std::string& shaderDirectory);

	private:
		void init(const std::string& directory);
		void reflect();
		bool compileShaders(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outShaderData);
		bool readDirectory(const std::string& directory, std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outShaderData);
		bool readFile(const std::filesystem::path& filepath, std::vector<uint32_t>& outShaderData);
		bool isFileType(const std::filesystem::path& filepath, const std::string& type);
	};
}


