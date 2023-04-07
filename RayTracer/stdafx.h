#pragma once

#include <windows.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <functional>

#define USE_PIX_RELEASE 1

#if defined _DEBUG || NDEBUG && USE_PIX_RELEASE
#define USE_PIX
#include "pix3.h" // https://devblogs.microsoft.com/pix/winpixeventruntime
#endif

#include "math/common.h"

#define DO_BENCHMARK 1
#include "processing/benchmark.h"

#define USE_FPEXCEPT _DEBUG // Use floating point exceptions. Remember to set /fp:except and /EHa in the compiler setting
#include "math/fpexcept.h"
#include "app/exceptions.h"
