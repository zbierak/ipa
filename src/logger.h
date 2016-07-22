#pragma once

#include <stdio.h>

#define LOG_LEVEL_ALL   0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_WARN  3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_NONE  5

// By default, all logging is enabled (you can change it in the cmake)
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

/**
 * Removes the path component from an absolute path name, leaving only the name after the
 * last slash.
 *
 * For instance, when you pass "/etc/foo/bar" to this function, it will return "bar".
 *
 * @param file_name the absolute file name from which the simple file name should be extracted.
 * This parameter must contain a NULL-terminated string and must not be NULL.
 * @return the file name after the last slash found in file_name
 */
const char* logger_sanitize_file_name(const char* file_name);

// Logger macros overwriting the log level check.
// They optionally include also the call location, but we need that info only in debug mode,
// since the errors (via LOG_ERROR) are presented to the end users on different events (e.g.
// whenever the device cannot be detected etc.), hence the need for the plain text messages
// for release.
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

// Logger macros which are enabled/disabled by the log level
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
