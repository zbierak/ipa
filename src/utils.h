#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// A null-safe variant of strdup
#define STRDUP(x) (((x) == NULL) ? NULL : strdup(x))

// A null-safe variant of strstr
#define STRSTR(haystack, needle) (((haystack) == NULL || (needle) == NULL) ? NULL : strstr(haystack, needle))

#define STREQ(a, b) (strcmp(a, b) == 0)
