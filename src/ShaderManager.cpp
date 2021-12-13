#include "pch.h"
#include "ShaderManager.h"

namespace cy3d
{
	ShaderManager::ShaderManager(VulkanContext& context) : _context(context)
	{

	}

	bool ShaderManager::add(const std::string& directory)
	{
		Ref<VulkanShader> shader = std::make_shared<VulkanShader>(_context, directory);
		_shaders[shader->getName()] = shader;
		return true;
	}

	Ref<VulkanShader> ShaderManager::get(const std::string& name)
	{
		CY_ASSERT(_shaders.count(name) != 0); //no shader by that name
		return _shaders.at(name);
	}

	const Ref<VulkanShader> ShaderManager::get(const std::string& name) const
	{
		CY_ASSERT(_shaders.count(name) != 0); //no shader by that name
		return _shaders.at(name);
	}
}
