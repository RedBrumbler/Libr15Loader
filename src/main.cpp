#include <cstdlib>
#include <string>
#include "_config.h"
#include "log.hpp"
#include "modloader.hpp"

#define EXPORT_FUNC extern "C" MAIN_EXPORT
void* original_r15_handle;

void* load_orig() {
  if (original_r15_handle) return original_r15_handle;
  original_r15_handle = DL_OPEN(ORIGINAL_R15, RTLD_NOW | RTLD_GLOBAL);
  if (!original_r15_handle) {
    LOG_ERROR("Could not find original r15 handle!");
  }
  return original_r15_handle;
}

#ifdef QUEST
#include <jni.h>

JNIEnv* jni_env = nullptr;

EXPORT_FUNC jint JNI_OnLoad(JavaVM* vm, void*) {
  LOG_INFO("Libr15Loader " GIT_BRANCH "(%X, modified: %d)", GIT_COMMIT, GIT_MODIFIED);

  jni_env = nullptr;

  LOG_INFO("JNI_OnLoad called, linking JNI methods");

  vm->AttachCurrentThread(&jni_env, nullptr);

  return JNI_VERSION_1_6;
}

EXPORT_FUNC void JNI_OnUnload(JavaVM* vm, void*) {
  LOG_INFO("JNI_OnUnload");
  modloader::unload(vm);
}

std::string GetNativeLibDir(JNIEnv* jenv) {
  auto activityThreadClass = jenv->FindClass("android/app/ActivityThread");
  auto activityThreadMethod =
      jenv->GetStaticMethodID(activityThreadClass, "currentActivityThread", "()Landroid/app/ActivityThread;");
  auto getApplicationMethod = jenv->GetMethodID(activityThreadClass, "getApplication", "()Landroid/app/Application;");
  auto contextClass = jenv->FindClass("android/content/Context");
  auto activityThread = jenv->CallStaticObjectMethod(activityThreadClass, activityThreadMethod);
  auto context = jenv->CallObjectMethod(activityThread, getApplicationMethod);
  auto fileClass = jenv->FindClass("java/io/File");
  auto getAbsolutePath = jenv->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");

  auto applicationInfoClass = jenv->FindClass("android/content/pm/ApplicationInfo");
  auto getApplicationInfoMethod =
      jenv->GetMethodID(contextClass, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
  auto applicationInfo = jenv->CallObjectMethod(context, getApplicationInfoMethod);
  auto sourceDirField = jenv->GetFieldID(applicationInfoClass, "nativeLibraryDir", "Ljava/lang/String;");
  auto sourceDir = jenv->GetObjectField(applicationInfo, sourceDirField);
  auto sourceDirjstring = reinterpret_cast<jstring>(sourceDir);
  auto sourceDirChars = jenv->GetStringUTFChars(sourceDirjstring, nullptr);
  std::string filesDir = { sourceDirChars };
  jenv->ReleaseStringUTFChars(sourceDirjstring, sourceDirChars);

  return filesDir;
}

EXPORT_FUNC void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
  LOG_INFO("ANativeActivity_onCreate");

  modloader::preload(jni_env, activity);

  auto nativeLibDir = GetNativeLibDir(jni_env);
  modloader::load(activity->env, nativeLibDir.c_str());

  auto handle = load_orig();
  if (!handle) std::exit(1);

  auto onCreate = reinterpret_cast<void (*)(ANativeActivity*, void*, size_t)>(
      DL_SYM(original_r15_handle, "ANativeActivity_onCreate"));
  if (!onCreate) std::exit(1);

  modloader::accept_r15_handle(activity->env, handle);

  onCreate(activity, savedState, savedStateSize);
}
#else
// TODO: PC code to invoke the modloader
#endif
