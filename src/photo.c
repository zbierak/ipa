#include "photo.h"
#include "utils.h"
#include "logger.h"

#include <glib.h>
#include <stdlib.h>

/**
 * The structure behind photo_h handle
 */
typedef struct photo_s
{
	char* file_name;			/// the file name of the photo (no path included)
	char* location;				/// the location of the photo, relative to the root directory for the corresponding device

	gint ref_count;				/// reference counter for photo_h
} photo_t;

photo_h photo_create(const char* file_name, const char* location)
{
	ASSERT_RET(file_name != NULL, NULL);
	ASSERT_RET(location != NULL, NULL);

	photo_h handle = (photo_h) calloc(1, sizeof(struct photo_s));
	ASSERT_RET(handle != NULL, NULL);

	handle->ref_count = 1;
	handle->file_name = strdup(file_name);
	handle->location = strdup(location);

	return handle;
}

const char* photo_get_file_name(const photo_h handle)
{
	ASSERT_RET(handle != NULL, NULL);
	return handle->file_name;
}

const char* photo_get_location(const photo_h handle)
{
	ASSERT_RET(handle != NULL, NULL);
	return handle->location;
}

photo_h photo_ref(photo_h handle)
{
	ASSERT_RET(handle, NULL);
	ASSERT_RET(handle->ref_count > 0, handle);

	g_atomic_int_inc(&handle->ref_count);
	return handle;
}

void photo_unref(photo_h handle)
{
	ASSERT_RET(handle);
	ASSERT_RET(handle->ref_count > 0);

	if (g_atomic_int_dec_and_test(&handle->ref_count))
	{
		free(handle->file_name);
		free(handle->location);
		free(handle);
	}
}
