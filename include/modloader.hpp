#pragma once

#include "modloader.hpp"
#ifdef QUEST
#include <jni.h>
#endif

#include <filesystem>
#include <string_view>
#include "_config.h"

namespace modloader {

using namespace std::string_view_literals;

MAIN_LOCAL extern void* modloaderHandle;

using preload_t = void(JNIEnv* env, char const* appId, char const* modloaderPath, char const* modloaderSourcePath,
                       char const* filesDir, char const* externalDir) noexcept;
MAIN_LOCAL constexpr auto preloadName = "modloader_preload"sv;
/// @brief called before the original r15 is dlopened
using load_t = void(JNIEnv* env, char const* soDir) noexcept;
MAIN_LOCAL constexpr auto loadName = "modloader_load"sv;
/// @brief called after the original r15 is dlopened and the activity is created
using accept_r15_handle_t = void(JNIEnv* env, void* r15Handle) noexcept;
MAIN_LOCAL constexpr auto accept_r15_handleName = "modloader_accept_r15_handle"sv;
/// @brief called during teardown (sometimes)
using unload_t = void(JavaVM* vm) noexcept;
MAIN_LOCAL constexpr auto unloadName = "modloader_unload"sv;

#ifdef QUEST
MAIN_LOCAL void preload(JNIEnv* env) noexcept;
MAIN_LOCAL void load(JNIEnv* env, char const* soDir) noexcept;
MAIN_LOCAL void accept_r15_handle(JNIEnv* env, void* r15Handle) noexcept;
MAIN_LOCAL void unload(JavaVM* vm) noexcept;
#else
MAIN_LOCAL void preload() noexcept;
MAIN_LOCAL void load(char const* soDir) noexcept;
MAIN_LOCAL void accept_r15_handle(void* r15Handle) noexcept;
MAIN_LOCAL void unload() noexcept;
#endif

}  // namespace modloader
