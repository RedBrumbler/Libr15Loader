# LibR15Loader
Acts as a proxy libr15 for loading in a modloader for EchoVR. Inspiration from this can be taken for other native android apps.

### Android (Quest)
When `ANativeActivity_onCreate` gets called on this library, it will load in a renamed original so all symbols exist. Then we will dlsym the correct onCreate call so the app is setup correctly. This allows us an entry point to load in a seperate modloader

### PC
TODO

# Usage
After building, rename the `libr15.so` in your echoVR apk to `libr15-original.so`, and place in the `libr15.so` that was built.

For android, then place a modloader at `/sdcard/ModData/com.readyatdawn.r15/ModLoader/*.so`
This binary will find the `.so` files in the ModLoader directory and load the first one as the modloader.

# ModLoader

A modloader should be `dlopen`-able without any dependencies. it should also be okay with being dlopend into the `GLOBAL` symbol lookup table. After that it may define the following (`extern C`) functions.

| Signature | Call point |
|-----------|------------|
| `void modloader_preload(JNIEnv* env, char const* appId, char const* modloaderPath, char const* modloaderSourcePath, char const* filesDir, char const* externalDir) noexcept`| This is called when the custom libr15 is originally dlopened by the main activity |
| `void modloader_load(JNIEnv* env, char const* soDir) noexcept`| Called before the original libr15.so is opened, but after preload |
| `void modloader_accept_r15_handle(JNIEnv* env, void* r15Handle) noexcept`| Called after libr15 is opened and the native activity is created |
| `void modloader_unload(JavaVM* vm) noexcept` | Called during teardown of the app, but not always |

TODO: pc modloader interactions, probably just remove the java arguments from the methods

# Resources
 - Inspired by [LibMainLoader](https://github.com/sc2ad/LibMainLoader/blob/master/README.md)
