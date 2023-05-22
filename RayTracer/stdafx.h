#pragma once

// Common library headers
#include <iostream>
#include <assert.h>

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

// Common project headers
#include "math/common.h"
