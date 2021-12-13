#include "pch.h"
#include "VulkanShader.h"
//
namespace cy3d
{
	VulkanShader::VulkanShader(const std::string& shaderDirectory)
	{
		init(shaderDirectory);
	}

	void VulkanShader::init(const std::string& directory)
	{
		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> binary{};
		readDirectory(directory);
		compile(binary);
		reflect(binary);
	}

	void VulkanShader::reflect(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& binary)
	{

		for (auto [stage, bin] : binary)
		{
			spirv_cross::Compiler compiler(bin);
			auto resources = compiler.get_shader_resources();

			for (const auto& resource : resources.uniform_buffers)
			{
				const auto& name = resource.name;
				auto& bufferType = compiler.get_type(resource.base_type_id);
				int memberCount = static_cast<uint32_t>(bufferType.member_types.size());
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_size(bufferType));
				CY_BASE_LOG_INFO("UBO -> name: {0} binding {1} desc set {2} size: {3}", name, binding, descriptorSet, size);
				CY_ASSERT(_descriptorSetsInfo.count(descriptorSet) == 0); //shouldn't have multiple ubos bound at the same descriptor set

				_descriptorSetsInfo[descriptorSet] = ShaderDescriptorSetInfo();
				ShaderUBOSetInfo uboInfo{};
				uboInfo.binding = binding;
				uboInfo.descriptorSet = descriptorSet;
				uboInfo.stage = stage;
				uboInfo.createInfo = BufferCreateInfo::createUBOInfo(size);
				_descriptorSetsInfo[descriptorSet].ubosInfo[name] = (uboInfo);
			}

			for (const auto& resource : resources.sampled_images)
			{
				//TODO what if UBO is in both the vertex and fragment shader.
				//Its stage should be VK_SHADER_STAGE_ALL but as of now if its both the vertex
				//and fragment shader it will be marked as VK_SHADER_STAGE_FRAGMENT.
				const auto& name = resource.name;
				auto& baseType = compiler.get_type(resource.base_type_id);
				auto& type = compiler.get_type(resource.type_id);
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				uint32_t dimension = baseType.image.dim;
				uint32_t arraySize = type.array[0];
				
				if (_descriptorSetsInfo.count(descriptorSet) == 0)
				{
					_descriptorSetsInfo[descriptorSet] = ShaderDescriptorSetInfo();
				}
				ShaderImageSamplerSetInfo imageSamplerInfo{};
				imageSamplerInfo.binding = binding;
				imageSamplerInfo.descriptorSet = descriptorSet;
				imageSamplerInfo.stage = stage;
				_descriptorSetsInfo[descriptorSet].imageSamplersInfo[name] = imageSamplerInfo;
				CY_BASE_LOG_INFO("Sampler -> name: {0} binding {1} desc set {2}", name, binding, descriptorSet);
			}
		}
		
	}

	bool VulkanShader::compile(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outShaderBinary)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		options.SetWarningsAsErrors();
		options.SetGenerateDebugInfo();

		for (auto [stage, shaderData] : _source)
		{
			shaderc::SpvCompilationResult spirvBinary = compiler.CompileGlslToSpv(shaderData.source, vkShaderStageToShaderCStage(stage), shaderData.filepath.string().c_str(), options);

			if (spirvBinary.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				CY_BASE_LOG_ERROR(spirvBinary.GetErrorMessage());
				CY_ASSERT(false);
			}
			const uint8_t* begin = reinterpret_cast<const uint8_t*>(spirvBinary.cbegin());
			const uint8_t* end = reinterpret_cast<const uint8_t*>(spirvBinary.cend());
			const ptrdiff_t size = end - begin;

			outShaderBinary[stage] = std::vector<uint32_t>(spirvBinary.cbegin(), spirvBinary.cend());
		}
		return true;
	}

	bool VulkanShader::readDirectory(const std::string& directory)
	{

		CY_ASSERT(std::filesystem::exists(directory));
		CY_ASSERT(std::filesystem::is_directory(directory));

		for (const auto& file : std::filesystem::directory_iterator(directory))
		{
			const auto& path = file.path();
			//set name to be the filename
			_name = path.filename().string();
			CY_BASE_LOG_INFO("Filename: {0}", path.filename().string());
			CY_BASE_LOG_INFO("Extension: {0}", path.extension().string());

			if (isFileType(path, VERT_EXTENSION))
			{
				CY_ASSERT(_source.count(VK_SHADER_STAGE_VERTEX_BIT) == 0); //multiple vert shaders cant be in the same directory
				ShaderData{ std::string(), path };
				_source[VK_SHADER_STAGE_VERTEX_BIT] = ShaderData{ std::string(), path };
				readFile(_source[VK_SHADER_STAGE_VERTEX_BIT]);
			}
			else if (isFileType(path, FRAG_EXTENSION))
			{
				CY_ASSERT(_source.count(VK_SHADER_STAGE_FRAGMENT_BIT) == 0); //multiple frag shaders cant be in the same directory
				_source[VK_SHADER_STAGE_FRAGMENT_BIT] = ShaderData{ std::string(), path };
				readFile(_source[VK_SHADER_STAGE_FRAGMENT_BIT]);
			}
			else
			{
				CY_BASE_LOG_ERROR("Extension not found: {0}", path.string());
				CY_ASSERT(false);
			}
		}
		return true;
	}

	bool VulkanShader::readFile(ShaderData& data)
	{
		std::string result;
		std::ifstream file(data.filepath, std::ios::in | std::ios::binary);
		if (file)
		{
			file.seekg(0, std::ios::end);
			result.resize(file.tellg());
			file.seekg(0, std::ios::beg);
			file.read(&result[0], result.size());
			data.source = result;
		}
		else
		{
			CY_BASE_LOG_ERROR("Failed to open file: {0}", data.filepath.string());
		}
		file.close();
		return true;

		/*FILE* file;
		errno_t err = fopen_s(&file, filepath.string().c_str(), "rb");
		if (!err)
		{
			fseek(file, 0, SEEK_END);
			uint64_t size = ftell(file);
			fseek(file, 0, SEEK_SET);
			outSource.resize(size / sizeof(uint32_t));
			fread(outShaderData.data(), sizeof(uint32_t), outShaderData.size(), file);
			fclose(file);
			return true;
		}	*/
		//CY_BASE_LOG_ERROR("Failed to open file: {0}", filepath.relative_path().string());
		//return false;
	}

	bool VulkanShader::isFileType(const std::filesystem::path& filepath, const std::string& type)
	{
		if (filepath.extension().string().find(type) != std::string::npos)
		{
			return true;
		}
		return false;
	}

	shaderc_shader_kind VulkanShader::vkShaderStageToShaderCStage(VkShaderStageFlagBits stage)
	{
		switch (stage)
		{
			case VK_SHADER_STAGE_VERTEX_BIT:    return shaderc_vertex_shader;
			case VK_SHADER_STAGE_FRAGMENT_BIT:  return shaderc_fragment_shader;
			case VK_SHADER_STAGE_COMPUTE_BIT:   return shaderc_compute_shader;
		}
		CY_ASSERT(false);
		return static_cast<shaderc_shader_kind>(0);
	}
}
