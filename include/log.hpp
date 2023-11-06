#pragma once

#define LOG_IDENTIFIER "[" LIB_ID "|" VERSION "]"

#if defined(QUEST)
// quest specific includes and logging
#include <android/log.h>
#include <android/native_app_glue/android_native_app_glue.h>

#define LOG_LEVEL(lvl, fmt, ...) __android_log_print(lvl, LOG_IDENTIFIER, fmt __VA_OPT__(, __VA_ARGS__))

#define LEVEL_INFO ANDROID_LOG_INFO
#define LEVEL_FATAL ANDROID_LOG_FATAL
#define LEVEL_ERROR ANDROID_LOG_ERROR
#define LEVEL_DEBUG ANDROID_LOG_DEBUG
#define LEVEL_WARN ANDROID_LOG_WARN
#define LEVEL_VERBOSE ANDROID_LOG_VERBOSE
#else
// pc specific includes and logging
#include <stdio.h>

#define LOG_LEVEL(lvl, fmt, ...) printf(lvl " " LOG_IDENTIFIER ": " fmt "\r\n" __VA_OPT__(, __VA_ARGS__))

#define LEVEL_INFO "I"
#define LEVEL_FATAL "C"
#define LEVEL_ERROR "E"
#define LEVEL_DEBUG "D"
#define LEVEL_WARN "W"
#define LEVEL_VERBOSE "V"
#endif

#define LOG_INFO(fmt, ...) LOG_LEVEL(LEVEL_INFO, fmt, __VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) LOG_LEVEL(LEVEL_FATAL, fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_LEVEL(LEVEL_ERROR, fmt, __VA_ARGS__)
#define LOG_DEBUG(fmt, ...) LOG_LEVEL(LEVEL_DEBUG, fmt, __VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG_LEVEL(LEVEL_WARN, fmt, __VA_ARGS__)
#define LOG_VERBOSE(fmt, ...) LOG_LEVEL(LEVEL_VERBOSE, fmt, __VA_ARGS__)
