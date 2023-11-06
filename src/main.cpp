#include <jni.h>
#include <cstdlib>
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
ANativeActivity* current_activity;

#ifdef QUEST
EXPORT_FUNC jint JNI_OnLoad(JavaVM* vm, void*) {
  JNIEnv* env = nullptr;

  LOG_INFO("JNI_OnLoad called, linking JNI methods");

  vm->AttachCurrentThread(&env, nullptr);

  modloader::preload(env);

  return JNI_VERSION_1_6;
}

EXPORT_FUNC void JNI_OnUnload(JavaVM* vm, void*) {
  LOG_INFO("JNI_OnUnload");
  modloader::unload(vm);
}

EXPORT_FUNC void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
  LOG_INFO("ANativeActivity_onCreate");
  char const* sopath = nullptr;  // TODO
  modloader::load(activity->env, sopath);

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
