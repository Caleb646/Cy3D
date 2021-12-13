#pragma once
#include "pch.h"
#include <spdlog/spdlog.h>


#define CY_BASE_LOG_INFO(...) spdlog::info(__VA_ARGS__)
#define CY_BASE_LOG_DEBUG(...) spdlog::debug(__VA_ARGS__)
#define CY_BASE_LOG_WARNING(...) spdlog::warn(__VA_ARGS__)
#define CY_BASE_LOG_ERROR(...) spdlog::error(__VA_ARGS__)
#define CY_BASE_LOG_CRTICAL(...) spdlog::critical(__VA_ARGS__)

#define CY_ASSERT(con) assert(con)
#define CY_NOCOPY(class_name) class_name(const class_name&) = delete;\
                              class_name& operator=(const class_name&) = delete\

namespace cy3d
{
	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using WeakRef = std::weak_ptr<T>;
}




