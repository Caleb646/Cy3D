#pragma once
#include "pch.h"

#include "../../core/core.h"

#define VK_CHECK(expression)		\
do {								\
	VkResult res = expression;		\
	CY_ASSERT(res == VK_SUCCESS);	\
} while(false);						\


namespace cy3d
{
	inline const char* VKResultToString(VkResult result);
}


