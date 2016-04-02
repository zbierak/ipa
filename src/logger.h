#pragma once

#include <stdio.h>

#define LOG_LEVEL_ALL   0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_WARN  3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_NONE  5

// by default, all logging is enabled (you can change it in the cmake)
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

const char* logger_sanitize_file_name(const char* file_name);

// logger macros overwriting the log level check
#ifdef LOG_MODE_DEBUG
#define LOG_DEBUG_OVERWRITE(format, ARG...) \
	do { printf("[DEBUG] " format " [%s@%s:%d]\n", ##ARG, __func__, logger_sanitize_file_name(__FILE__), __LINE__); } while(0)
#define LOG_INFO_OVERWRITE(format, ARG...) \
	do { printf("[INFO]  " format " [%s@%s:%d]\n", ##ARG, __func__, logger_sanitize_file_name(__FILE__), __LINE__); } while(0)
#define LOG_WARN_OVERWRITE(format, ARG...) \
	do { printf("[WARN]  " format " [%s@%s:%d]\n", ##ARG, __func__, logger_sanitize_file_name(__FILE__), __LINE__); } while(0)
#define LOG_ERROR_OVERWRITE(format, ARG...) \
	do { fprintf(stderr, "[ERROR] " format " [%s@%s:%d]\n", ##ARG, __func__, logger_sanitize_file_name(__FILE__), __LINE__); } while(0)
#else
#define LOG_DEBUG_OVERWRITE(format, ARG...) do { printf(format "\n", ##ARG); } while(0)
#define LOG_INFO_OVERWRITE(format, ARG...) 	do { printf(format "\n", ##ARG); } while(0)
#define LOG_WARN_OVERWRITE(format, ARG...) 	do { printf(format "\n", ##ARG); } while(0)
#define LOG_ERROR_OVERWRITE(format, ARG...) do { fprintf(stderr, format "\n", ##ARG); } while(0)
#endif

// logger macros which are enabled/disabled by the log level
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(format, ARG...) LOG_DEBUG_OVERWRITE(format, ##ARG)
#else
#define LOG_DEBUG(format, ARG...);
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(format, ARG...) LOG_INFO_OVERWRITE(format, ##ARG)
#else
#define LOG_INFO(format, ARG...);
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(format, ARG...) LOG_WARN_OVERWRITE(format, ##ARG)
#else
#define LOG_WARN(format, ARG...);
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(format, ARG...) LOG_ERROR_OVERWRITE(format, ##ARG)
#else
#define LOG_ERROR(format, ARG...);
#endif
