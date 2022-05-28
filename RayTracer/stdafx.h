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

#include "common.h"
#include "benchmark.h"