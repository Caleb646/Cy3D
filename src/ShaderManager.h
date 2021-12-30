#pragma once
#include "pch.h"
#include "core/core.h"
#include "platform/Vulkan/VulkanContext.h"
#include "platform/Vulkan/VulkanShader.h"

namespace cy3d
{
	class ShaderManager
	{
	private:
		VulkanContext& _context;
		std::unordered_map<std::string, Ref<VulkanShader>> _shaders;
	public:
		ShaderManager(VulkanContext& context);
		bool add(const std::string& directory, const std::string& name);
		Ref<VulkanShader> get(const std::string& name);
		const Ref<VulkanShader> get(const std::string& name) const;
	};
}


