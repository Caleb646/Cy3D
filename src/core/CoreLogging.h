#pragma once
#include "pch.h"
#include <spdlog/spdlog.h>


#define CY_BASE_LOG_INFO(...) spdlog::info(__VA_ARGS__)
#define CY_BASE_LOG_DEBUG(...) spdlog::debug(__VA_ARGS__)
#define CY_BASE_LOG_WARNING(...) spdlog::warn(__VA_ARGS__)
#define CY_BASE_LOG_ERROR(...) spdlog::error(__VA_ARGS__)
#define CY_BASE_LOG_CRTICAL(...) spdlog::critical(__VA_ARGS__)
