/** @file Vulkan Context C++ header file 
 * @date 28th of August 2026
 * @note This is for the includes directories of the needed API's and definitions for the Driver.
 */

#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#if defined(__GNUC__) && !defined(__clang__)
#define _GPLUSPLUS
#endif

#if defined(__clang__)
#define _CLANGPP
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define MS_CPLUSPLUS
#endif