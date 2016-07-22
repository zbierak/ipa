#pragma once

#include <string.h>

// A null-safe variant of strdup
#define STRDUP(x) (((x) == NULL) ? NULL : strdup(x))

// A null-safe variant of strstr
#define STRSTR(haystack, needle) (((haystack) == NULL || (needle) == NULL) ? NULL : strstr(haystack, needle))
