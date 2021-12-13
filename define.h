#pragma once
#include <pch.h>

#define CY_ASSERT(con) assert(con)

#define CY_NOCOPY(class_name) class_name(const class_name&) = delete;\
                              class_name& operator=(const class_name&) = delete\

#define CY_MOVE std::move
