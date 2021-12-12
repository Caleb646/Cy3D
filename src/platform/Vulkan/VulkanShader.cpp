#include "pch.h"
#include "VulkanShader.h"
//
namespace cy3d
{
	VulkanShader::VulkanShader(VulkanContext& context, const std::string& shaderDirectory) : _context(context)
	{
		init(shaderDirectory);
	}

	void VulkanShader::init(const std::string& directory)
	{
		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> shaderData{};
		readDirectory(directory, shaderData);
	}

	void VulkanShader::reflect()
	{
		std::vector<uint32_t> spirv_binary = std::vector<uint32_t>();// load_spirv_file();

		spirv_cross::CompilerGLSL glsl(std::move(spirv_binary));

		// The SPIR-V is now parsed, and we can perform reflection on it.
		spirv_cross::ShaderResources resources = glsl.get_shader_resources();

		// Get all sampled images in the shader.
		for (auto& resource : resources.sampled_images)
		{
			unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
			unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
			printf("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);

			// Modify the decoration to prepare it for GLSL.
			glsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);

			// Some arbitrary remapping if we want.
			glsl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
		}

		// Set some options.
		spirv_cross::CompilerGLSL::Options options;
		options.version = 310;
		options.es = true;
		glsl.set_common_options(options);

		// Compile to GLSL, ready to give to GL driver.
		std::string source = glsl.compile();
	}

	bool VulkanShader::compileShaders(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outShaderData)
	{
		/*shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		options.SetWarningsAsErrors();
		options.SetGenerateDebugInfo();*/

		for (auto [shaderStage, source] : outShaderData)
		{

		}
		return true;
	}

	bool VulkanShader::readDirectory(const std::string& directory, std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outShaderData)
	{

		CY_ASSERT(std::filesystem::exists(directory));
		CY_ASSERT(std::filesystem::is_directory(directory));

		for (const auto& file : std::filesystem::directory_iterator(directory))
		{
			auto& path = file.path();
			LOG_INFO((const char*)path.filename().c_str());
			LOG_INFO((const char*)path.extension().c_str());

			if (isFileType(path, VERT_EXTENSION))
			{
				CY_ASSERT(outShaderData.count(VK_SHADER_STAGE_VERTEX_BIT) == 0); //multiple vert shaders cant be in the same directory
				outShaderData[VK_SHADER_STAGE_VERTEX_BIT] = std::vector<uint32_t>();
				readFile(path, outShaderData[VK_SHADER_STAGE_VERTEX_BIT]);
			}
			else if (isFileType(path, FRAG_EXTENSION))
			{
				CY_ASSERT(outShaderData.count(VK_SHADER_STAGE_FRAGMENT_BIT) == 0); //multiple vert shaders cant be in the same directory
				outShaderData[VK_SHADER_STAGE_FRAGMENT_BIT] = std::vector<uint32_t>();
				readFile(path, outShaderData[VK_SHADER_STAGE_FRAGMENT_BIT]);
			}
			else
			{
				CY_ASSERT(false); //unsupported shader type in directory
			}
		}
		return true;
	}

	bool VulkanShader::readFile(const std::filesystem::path& filepath, std::vector<uint32_t>& outShaderData)
	{
		FILE* file;
		errno_t err = fopen_s(&file, (const char*)filepath.relative_path().c_str(), "rb");
		if (!err)
		{
			fseek(file, 0, SEEK_END);
			uint64_t size = ftell(file);
			fseek(file, 0, SEEK_SET);
			outShaderData.resize(size / sizeof(uint32_t));
			fread(outShaderData.data(), sizeof(uint32_t), outShaderData.size(), file);
			fclose(file);
		}
		else
		{
			LOG_ERROR(err + "");
		}
		return true;
	}

	bool VulkanShader::isFileType(const std::filesystem::path& filepath, const std::string& type)
	{
		if (filepath.filename().string().find(type) != std::string::npos)
		{
			return true;
		}
		LOG_WARNING("Extension not found.");
		return false;
	}
}
