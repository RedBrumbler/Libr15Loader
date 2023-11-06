// MIT License

// Copyright (c) 2019 Anairkoen Schno

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "fileutils.hpp"

#include <android/native_activity.h>
#include <ctype.h>
#include <jni.h>

#include <cstring>
#include <filesystem>
#include <string_view>
#include <thread>

#include "log.hpp"

namespace {
char* trimWhitespace(char* str) {
  char* end;
  while (isspace((unsigned char)*str)) str++;
  if (*str == 0) return str;

  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) end--;

  end[1] = '\0';

  return str;
}

#define STRIFY(...) STRIFY2(__VA_ARGS__)
#define STRIFY2(...) #__VA_ARGS__

#ifndef NO_SPECIFIC_FAIL_LOGS
#define ERR_CHECK(val, ...)                                                \
  auto val = __VA_ARGS__;                                                  \
  if (val == nullptr) {                                                    \
    LOG_ERROR("Failed: " __FILE__ ":" STRIFY(__LINE__) ": " #__VA_ARGS__); \
    return {};                                                             \
  }
#else
#define ERR_CHECK(val, ...)                              \
  auto val = __VA_ARGS__;                                \
  if (val == nullptr) {                                  \
    LOG_ERROR("Failed: " __FILE__ ":" STRIFY(__LINE__)); \
    return {};                                           \
  }
#endif

bool ensurePermsWithAppId(JNIEnv* env, ANativeActivity* activity, std::string_view application_id) {
  ERR_CHECK(clazz, env->GetObjectClass(activity->clazz));
  ERR_CHECK(intentClass, env->FindClass("android/content/Intent"));
  ERR_CHECK(intentCtorID, env->GetMethodID(intentClass, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V"));
  ERR_CHECK(startActivity, env->GetMethodID(clazz, "startActivity", "(Landroid/content/Intent;)V"));
  ERR_CHECK(uriClass, env->FindClass("android/net/Uri"));
  ERR_CHECK(uriParse, env->GetStaticMethodID(uriClass, "parse", "(Ljava/lang/String;)Landroid/net/Uri;"));
  ERR_CHECK(envClass, env->FindClass("android/os/Environment"));
  ERR_CHECK(isExternalMethod, env->GetStaticMethodID(envClass, "isExternalStorageManager", "()Z"));

  using namespace std::chrono_literals;
  // We loop the attempt to display permissions
  // The goal here is to avoid getting in too early
  // However, we don't really know what will happen:
  // 1. If a user does not select anything for the duration of the delay
  // 2. If the delay is too excessive and causes the app to exit out from watchdog
  constexpr static auto kNumTries = 3;
  constexpr static auto kDelay = 5000ms;

  for (int i = 0; i < kNumTries; i++) {
    auto result = env->CallStaticBooleanMethod(envClass, isExternalMethod);
    if (env->ExceptionCheck()) {
      LOG_ERROR("Failed: to call Environment.getIsExternalStorageManager()");
      return false;
    }
    LOG_INFO("Fetch of isExternalStorageManager(): %d", result);
    // If we already granted permissions, good to go
    if (result) return true;
    // Otherwise, need to display
    LOG_DEBUG("Attempt to show intent for MANAGE APP ALL FILES ACCESS PERMISSION:");
    // We pay the penalty of reconstructing these every iteration, as we don't expect to need to do so.
    const jstring actionName = env->NewStringUTF("android.settings.MANAGE_APP_ALL_FILES_ACCESS_PERMISSION");
    const jstring packageName = env->NewStringUTF((std::string("package:") + application_id.data()).c_str());
    ERR_CHECK(uri, env->CallStaticObjectMethod(uriClass, uriParse, packageName));
    ERR_CHECK(intent, env->NewObject(intentClass, intentCtorID, actionName, uri));
    // At this point, sleep the thread to avoid acting too fast
    env->CallVoidMethod(activity->clazz, startActivity, intent);
    if (env->ExceptionCheck()) return false;
    std::this_thread::sleep_for(kDelay);
  }
  return true;
}

}  // namespace

std::optional<fileutils::path_container> fileutils::getDirs(JNIEnv* env, std::string_view application_id) {
  ERR_CHECK(activityThreadClass, env->FindClass("android/app/ActivityThread"));
  ERR_CHECK(currentActivityThreadMethod,
            env->GetStaticMethodID(activityThreadClass, "currentActivityThread", "()Landroid/app/ActivityThread;"));
  ERR_CHECK(contextClass, env->FindClass("android/content/Context"));
  ERR_CHECK(getApplicationMethod,
            env->GetMethodID(activityThreadClass, "getApplication", "()Landroid/app/Application;"));
  ERR_CHECK(filesDirMethod, env->GetMethodID(contextClass, "getFilesDir", "()Ljava/io/File;"));
  ERR_CHECK(externalDirMethod,
            env->GetMethodID(contextClass, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;"));
  ERR_CHECK(fileClass, env->FindClass("java/io/File"));
  ERR_CHECK(absDirMethod, env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;"));

  using namespace fileutils::literals;
  // Construct the path to the modloader here, as there's no point in attempting to load if we can't get the other dirs
  // as well
  fileutils::path_container result{ .modloaderSearchPath = std::filesystem::absolute("/sdcard/ModData"_fp /
                                                                                     application_id / "Modloader") };

  ERR_CHECK(at, env->CallStaticObjectMethod(activityThreadClass, currentActivityThreadMethod));
  ERR_CHECK(context, env->CallObjectMethod(at, getApplicationMethod));
  ERR_CHECK(filesDir, env->CallObjectMethod(context, filesDirMethod));
  {
    ERR_CHECK(str, reinterpret_cast<jstring>(env->CallObjectMethod(filesDir, absDirMethod)));
    auto chars = env->GetStringUTFChars(str, nullptr);
    result.filesDir = chars;
    env->ReleaseStringUTFChars(str, chars);
  }

  ERR_CHECK(externalDir, env->CallObjectMethod(context, externalDirMethod, static_cast<jstring>(nullptr)));
  {
    ERR_CHECK(str, reinterpret_cast<jstring>(env->CallObjectMethod(externalDir, absDirMethod)));
    auto chars = env->GetStringUTFChars(str, nullptr);
    result.externalDir = chars;
    env->ReleaseStringUTFChars(str, chars);
  }
  return result;
}

#undef ERR_CHECK

std::optional<std::string> fileutils::getApplicationId() {
  FILE* cmdline = fopen("/proc/self/cmdline", "r");
  if (cmdline) {
    // not sure what the actual max is, but path_max should cover it
    char application_id[PATH_MAX] = { 0 };
    fread(application_id, sizeof(application_id), 1, cmdline);
    fclose(cmdline);
    trimWhitespace(application_id);
    return std::string(application_id);
  }
  return std::nullopt;
}

bool fileutils::ensurePerms(JNIEnv* env, ANativeActivity* activity, std::string_view application_id) {
  // TODO: The correct thing to do would be to listen to the completion event and resume modloading when that happens.
  // Instead, we actually sleep the thread, which is potentially problematic.
  if (ensurePermsWithAppId(env, activity, application_id)) return true;

  if (env->ExceptionCheck()) env->ExceptionDescribe();
  LOG_ERROR("libmain.ensurePerms failed! See 'System.err' tag.");
  env->ExceptionClear();
  return false;
}
