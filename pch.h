#pragma once


#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include <chrono>
#include <functional>
#include <cassert>
#include <filesystem>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#define M3D_FORCE_COLUMN_MAJOR_ORDERING
#define M3D_FORCE_DEPTH_ZERO_TO_ONE
#include <M3D/M3D.h>


#include <Logi/Logi.h>
