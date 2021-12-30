#include "pch.h"
#include "VulkanShader.h"
#include "VulkanDevice.h"
//
namespace cy3d
{
	VulkanShader::VulkanShader(VulkanContext& context, const std::string& shaderDirectory, const std::string& name) : _context(context), _name(name)
	{
		init(shaderDirectory);
	}

	void VulkanShader::init(const std::string& directory)
	{
		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> binary{};

		if (needsRecompiled(directory))
		{
			readSourceDirectory(directory);
			compile(directory, binary);
		}
		else
		{
			readBinaryDirectory(directory, binary);
		}	
		reflect(binary);
		createShaderModules(binary);
		createDescriptorSetLayouts();
	}

	bool VulkanShader::createDescriptorSetLayouts()
	{
		_descriptorSetLayouts.resize(_descriptorSetsInfo.size());
		for (const auto [setId, setMap] : _descriptorSetsInfo)
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings{};

			for (const auto [uboName, uboInfo] : setMap.ubosInfo)
			{
				VkDescriptorSetLayoutBinding bindingInfo{};
				bindingInfo.binding = uboInfo.binding;
				bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bindingInfo.descriptorCount = setMap.ubosInfo.size();
				bindingInfo.stageFlags = uboInfo.stage;
				bindingInfo.pImmutableSamplers = nullptr; // Optional
				//CY_ASSERT(_bindings.count(uboInfo.binding) == 0);
				//_bindings[uboInfo.binding] = bindingInfo;
				bindings.push_back(bindingInfo);
			}

			for (const auto [samplerName, samplerInfo] : setMap.imageSamplersInfo)
			{
				//TODO need to make sure duplicate bindings raise an error
				VkDescriptorSetLayoutBinding bindingInfo{};
				bindingInfo.binding = samplerInfo.binding;
				bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bindingInfo.descriptorCount = setMap.imageSamplersInfo.size();
				bindingInfo.stageFlags = samplerInfo.stage;
				bindingInfo.pImmutableSamplers = nullptr; // Optional
				//CY_ASSERT(_bindings.count(samplerInfo.binding) == 0);
				//_bindings[samplerInfo.binding] = bindingInfo;
				bindings.push_back(bindingInfo);
			}

			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();
			VK_CHECK(vkCreateDescriptorSetLayout(_context.getDevice()->device(), &layoutInfo, nullptr, &_descriptorSetLayouts[setId]));
		}

		return true;
	}

	bool VulkanShader::createShaderModules(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& binary)
	{
		for (auto [stage, bin] : binary)
		{
			CY_ASSERT(bin.size() != 0);
			VkShaderModuleCreateInfo moduleCreateInfo{};
			moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.codeSize = bin.size() * sizeof(uint32_t);
			moduleCreateInfo.pCode = bin.data();

			VkShaderModule shaderModule;
			VK_CHECK(vkCreateShaderModule(_context.getDevice()->device(), &moduleCreateInfo, NULL, &shaderModule));

			VkPipelineShaderStageCreateInfo shaderStage{};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = stage;
			shaderStage.module = shaderModule;
			shaderStage.pName = "main";

			_pipelineCreateInfo.push_back(std::move(shaderStage));
		}

		return true;
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
				//CY_ASSERT(_descriptorSetsInfo.count(descriptorSet) == 0); //shouldn't have multiple ubos bound at the same descriptor set
				if (_descriptorSetsInfo.count(descriptorSet) == 0)
				{
					_descriptorSetsInfo[descriptorSet] = ShaderDescriptorSetInfo();
				}
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

	bool VulkanShader::compile(const std::string& directory, std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outShaderBinary)
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
			const std::byte* begin = reinterpret_cast<const std::byte*>(spirvBinary.cbegin());
			const std::byte* end = reinterpret_cast<const std::byte*>(spirvBinary.cend());
			const ptrdiff_t size = end - begin;

			outShaderBinary[stage] = std::vector<uint32_t>(spirvBinary.cbegin(), spirvBinary.cend());
		}

		auto shaderCacheDir = directory + "/" + SHADER_BIN_FOLDER;
		if (!(std::filesystem::exists(shaderCacheDir)))
		{
			std::filesystem::create_directory(shaderCacheDir);
			CY_BASE_LOG_INFO("Created shader cache directory: {0}", shaderCacheDir);
		}

		for (auto& [stage, binary] : outShaderBinary)
		{
			if (stage == VK_SHADER_STAGE_VERTEX_BIT)
			{
				cacheBinary(shaderCacheDir + "/" + _name + ".vert.spv", binary);
			}
			else if (stage == VK_SHADER_STAGE_FRAGMENT_BIT)
			{
				cacheBinary(shaderCacheDir + "/" + _name + ".frag.spv", binary);
			}
			else
			{
				CY_BASE_LOG_INFO("Cannot cache shader binary of unknown shader stage: {0}", stage);
			}
		}
		return true;
	}

	bool VulkanShader::cacheBinary(const std::string& path, std::vector<uint32_t> binary)
	{
		std::ofstream fout(path, std::ios::out | std::ios::binary);
		fout.write(reinterpret_cast<const char*>(binary.data()), binary.size() * sizeof(uint32_t));
		fout.close();
		CY_BASE_LOG_INFO("Cached shader to {0}", path);
		return true;
	}

	bool VulkanShader::needsRecompiled(const std::string& directory)
	{
		auto shaderCacheDir = directory + "/" + SHADER_BIN_FOLDER;
		if (!(std::filesystem::exists(shaderCacheDir)))
		{
			CY_BASE_LOG_INFO("Shader cache directory does not exist so: {0} will be recompiled.", _name);
			return true;
		}

		std::chrono::time_point<std::filesystem::_File_time_clock>::clock::duration maxShaderSourceModifiedTime{};
		bool first{ true };
		for (const auto& file : std::filesystem::directory_iterator(directory))
		{
			auto modTime = file.last_write_time().time_since_epoch();
			if (first)
			{
				maxShaderSourceModifiedTime = modTime;
				first = false;
			}

			if (modTime > maxShaderSourceModifiedTime)
			{
				maxShaderSourceModifiedTime = modTime;
			}
		}

		for (const auto& file : std::filesystem::directory_iterator(shaderCacheDir))
		{
			auto modTime = file.last_write_time().time_since_epoch();
			if (modTime < maxShaderSourceModifiedTime)
			{
				CY_BASE_LOG_INFO("Shader {0} will be recompiled.", _name);
				return true;
			}
		}
		return false;
	}

	bool VulkanShader::readSourceDirectory(const std::string& directory)
	{
		CY_ASSERT(std::filesystem::exists(directory));
		CY_ASSERT(std::filesystem::is_directory(directory));

		for (const auto& file : std::filesystem::directory_iterator(directory))
		{
			const auto& path = file.path();
			//set name to be the filename it will be strimmed later
			//_name = path.filename().string();
			CY_BASE_LOG_INFO("Filename: {0}", path.filename().string());
			if (file.is_directory())
			{
				continue;
			}
			if (isFileType(path, VERT_EXTENSION))
			{
				CY_ASSERT(_source.count(VK_SHADER_STAGE_VERTEX_BIT) == 0); //multiple vert shaders cant be in the same directory
				ShaderData{ std::string(), path };
				_source[VK_SHADER_STAGE_VERTEX_BIT] = ShaderData{ std::string(), path };
				readSource(_source[VK_SHADER_STAGE_VERTEX_BIT]);
			}
			else if (isFileType(path, FRAG_EXTENSION))
			{
				CY_ASSERT(_source.count(VK_SHADER_STAGE_FRAGMENT_BIT) == 0); //multiple frag shaders cant be in the same directory
				_source[VK_SHADER_STAGE_FRAGMENT_BIT] = ShaderData{ std::string(), path };
				readSource(_source[VK_SHADER_STAGE_FRAGMENT_BIT]);
			}
			else
			{
				CY_BASE_LOG_ERROR("Extension not found: {0}", path.string());
				CY_ASSERT(false);
			}
		}
		//remove extension from shader name
		//stripFilenameExtension();
		return true;
	}

	bool VulkanShader::readBinaryDirectory(const std::string& directory, std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& binary)
	{
		auto shaderCacheDir = directory + "/" + SHADER_BIN_FOLDER;
		CY_ASSERT(std::filesystem::exists(shaderCacheDir));
		CY_ASSERT(std::filesystem::is_directory(shaderCacheDir));

		for (const auto& file : std::filesystem::directory_iterator(shaderCacheDir))
		{
			const auto& path = file.path();
			CY_BASE_LOG_INFO("Filename: {0}", path.filename().string());
			if (file.is_directory())
			{
				continue;
			}
			if (isFileType(path, VERT_EXTENSION))
			{
				CY_ASSERT(binary.count(VK_SHADER_STAGE_VERTEX_BIT) == 0); //multiple vert shaders cant be in the same directory
				ShaderData{ std::string(), path };
				binary[VK_SHADER_STAGE_VERTEX_BIT] = std::vector<uint32_t>();
				readBinary(path.string(), binary[VK_SHADER_STAGE_VERTEX_BIT]);
			}
			else if (isFileType(path, FRAG_EXTENSION))
			{
				CY_ASSERT(binary.count(VK_SHADER_STAGE_FRAGMENT_BIT) == 0); //multiple frag shaders cant be in the same directory
				binary[VK_SHADER_STAGE_FRAGMENT_BIT] = std::vector<uint32_t>();
				readBinary(path.string(), binary[VK_SHADER_STAGE_FRAGMENT_BIT]);
			}
			else
			{
				CY_BASE_LOG_ERROR("Extension not found: {0}", path.string());
				CY_ASSERT(false);
			}
		}
		return true;
	}

	bool VulkanShader::readSource(ShaderData& data)
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
	}

	bool VulkanShader::readBinary(const std::string& filepath, std::vector<uint32_t>& outBinary)
	{
		FILE* file;
		errno_t err = fopen_s(&file, filepath.c_str(), "rb");
		if (!err)
		{
			fseek(file, 0, SEEK_END);
			uint64_t size = ftell(file);
			fseek(file, 0, SEEK_SET);
			outBinary.resize(size / sizeof(uint32_t));
			fread(outBinary.data(), sizeof(uint32_t), outBinary.size(), file);
			fclose(file);
			return true;
		}
		CY_BASE_LOG_ERROR("Failed to open file: {0}", filepath);
		return false;
	}

	bool VulkanShader::isFileType(const std::filesystem::path& filepath, const std::string& type)
	{
		//auto ext = filepath.string();
		if (filepath.string().find(type) != std::string::npos)
		{
			return true;
		}
		return false;
	}

	//bool VulkanShader::stripFilenameExtension()
	//{
	//	std::size_t index = _name.find(".");
	//	CY_ASSERT(index != std::string::npos);
	//	_name = std::move(_name.substr(0, index));
	//	return true;
	//}

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
