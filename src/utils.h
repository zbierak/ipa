#pragma once

#include <string.h>

#define STRDUP(x) (((x) == NULL) ? NULL : strdup(x))

#define STRSTR(haystack, needle) (((haystack) == NULL || (needle) == NULL) ? NULL : strstr(haystack, needle))
