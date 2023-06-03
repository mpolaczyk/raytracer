#pragma once

//// Common library headers
#include <string>
#include <vector>

// Build type detection
#ifdef _DEBUG
#define BUILD_DEBUG 1
#define BUILD_RELEASE 0
#elif NDEBUG
#define BUILD_DEBUG 0
#define BUILD_RELEASE 1
#endif

// Defines per build configuration
#if BUILD_DEBUG
#define USE_BENCHMARK 1
#define USE_PIX 1
#define USE_SIMD 1
#define USE_FPEXCEPT 1 // Use floating point exceptions. Remember to set /fp:except and /EHa in the compiler setting
#elif BUILD_RELEASE
#define USE_BENCHMARK 1
#define USE_PIX 1
#define USE_SIMD 1
#define USE_FPEXCEPT 0
#endif

// Third party defines
#define TINYOBJLOADER_IMPLEMENTATION

#define IMGUI_DISABLE_DEMO_WINDOWS
#define IMGUI_DISABLE_METRICS_WINDOW

#define IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
#define IMGUI_ENABLE_WIN32_DEFAULT_IME_FUNCTIONS
#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#define IMGUI_DISABLE_WIN32_FUNCTIONS
#define IMGUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#define IMGUI_DISABLE_FILE_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS

#define SPDLOG_USE_STD_FORMAT
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

// Common project headers
#include "math/common.h"