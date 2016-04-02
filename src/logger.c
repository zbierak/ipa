#include "logger.h"

#include <string.h>

const char* logger_sanitize_file_name(const char* file_name)
{
	int last_pos = -1;
	int i = 0;

	while (file_name[i] != 0)
	{
		if (file_name[i] == '/')
			last_pos = i;
		i++;
	}

	return &file_name[last_pos + 1];
}

