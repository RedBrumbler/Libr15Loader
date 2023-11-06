#pragma once

#define MAIN_EXPORT __attribute__((visibility("default")))
#define MAIN_LOCAL __attribute__((visibility("hidden")))

// Helper defines for reducing codesize
#ifdef NO_EXCESS_LOGGING
#define NO_SPECIFIC_FAIL_LOGS
#define NO_STAT_DUMPS
#endif

#define LOG_IDENTIFIER "[" LIB_ID "|" VERSION "]"

#if defined(LINUX) || defined(QUEST)
#include <dlfcn.h>
#define ORIGINAL_R15 "libr15-original.so"

#define DL_OPEN(lib, flags) (dlopen(lib, flags))
#define DL_SYM(handle, symbol) (dlsym(handle, symbol))

#elif defined(WINDOWS)
// TODO: find the windows equivalent for dlopen
#define ORIGINAL_R15 "libr15-original.dll"

#define DL_OPEN(lib, flags)
#define DL_SYM(handle, symbol)

#else
#warning Please define a known build target for the loader
#endif
