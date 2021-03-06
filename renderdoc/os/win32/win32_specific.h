/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020 Baldur Karlsson
 * Copyright (c) 2014 Crytek
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <intrin.h>
#include <stdlib.h>
#include <windows.h>
#include "data/resource.h"

#define EndianSwap16(x) _byteswap_ushort(x)
#define EndianSwap32(x) _byteswap_ulong(x)
#define EndianSwap64(x) _byteswap_uint64(x)

#define EmbeddedResourceType int
#define EmbeddedResource(filename) CONCAT(RESOURCE_, filename)

#define GetEmbeddedResource(filename) GetDynamicEmbeddedResource(EmbeddedResource(filename))
rdcstr GetDynamicEmbeddedResource(int resource);

namespace OSUtility
{
inline void ForceCrash()
{
  *((int *)NULL) = 0;
}
inline void DebugBreak()
{
  __debugbreak();
}
inline bool DebuggerPresent()
{
  return ::IsDebuggerPresent() == TRUE;
}
void WriteOutput(int channel, const char *str);
};

namespace Threading
{
typedef CriticalSectionTemplate<CRITICAL_SECTION> CriticalSection;
typedef RWLockTemplate<SRWLOCK> RWLock;
};

namespace Bits
{
inline uint32_t CountLeadingZeroes(uint32_t value)
{
  DWORD index;
  BOOLEAN result = _BitScanReverse(&index, value);
  return (result == TRUE) ? (index ^ 31) : 32;
}

#if ENABLED(RDOC_X64)
inline uint64_t CountLeadingZeroes(uint64_t value)
{
  DWORD index;
  BOOLEAN result = _BitScanReverse64(&index, value);
  return (result == TRUE) ? (index ^ 63) : 64;
}
#endif

inline uint32_t CountTrailingZeroes(uint32_t value)
{
  DWORD index;
  BOOLEAN result = _BitScanForward(&index, value);
  return (result == TRUE) ? index : 32;
}

#if ENABLED(RDOC_X64)
inline uint64_t CountTrailingZeroes(uint64_t value)
{
  DWORD index;
  BOOLEAN result = _BitScanForward64(&index, value);
  return (result == TRUE) ? index : 64;
}
#endif

inline uint32_t CountOnes(uint32_t value)
{
#if defined(_M_ARM) || defined(_M_ARM64)
  // __popcnt instrinsic is not available on ARM.
  // https://bajamircea.github.io/coding/cpp/2017/10/10/counting-bits-better-groups.html
  // "8.6 Efficient Implementation of Population-Count Function in 32-Bit Mode"
  value -= (value >> 1) & 0x55555555;                            // 5 is 0101 in binary
  value = (value & 0x33333333) + ((value >> 2) & 0x33333333);    // 3 is 0011 in binary
  value = (value + (value >> 4)) & 0x0f0f0f0f;
  value *= 0x1010101;    // magic multiplier
  value >>= 24;
  return value;
#else
  return __popcnt(value);
#endif
}

#if ENABLED(RDOC_X64)
inline uint64_t CountOnes(uint64_t value)
{
#if defined(_M_ARM) || defined(_M_ARM64)
  // TODO: Port 32-bit efficient implementation to 64-bit if needed.
  uint32_t count = 0;
  for(; value > 0; value >>= 1)
  {
    count += (value & 1);
  }
  return count;
#else
  return __popcnt64(value);
#endif
}
#endif
};    // namespace Bits
