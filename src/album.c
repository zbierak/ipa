#include "album.h"
#include "utils.h"
#include "logger.h"

#include <glib.h>

/**
 * A structure behind album_h handle
 */
typedef struct album_s
{
	char* name;             /// the name of the album
	GHashTable* photos;     /// the lookup table of all photos <photo-name, photo-details> [char*,photo_h]

	gint ref_count;         /// reference counter for album_h
} album_t;

album_h album_create(const char* name)
{
	ASSERT_RET(name != NULL, NULL);

	album_h handle = (album_h) calloc(1, sizeof(struct album_s));
	ASSERT_RET(handle != NULL, NULL);

	handle->ref_count = 1;
	handle->name = strdup(name);
	handle->photos = g_hash_table_new_full(g_str_hash, g_str_equal, free, (GDestroyNotify) photo_unref);

	return handle;
}

const char* album_get_name(const album_h handle)
{
	ASSERT_RET(handle != NULL, NULL);
	return handle->name;
}

bool album_add_photo(album_h handle, photo_h photo)
{
	ASSERT_RET(handle != NULL, false);
	ASSERT_RET(photo != NULL, false);
	ASSERT_RET(photo_get_file_name(photo) != NULL, false);

#ifdef ENABLE_DEBUG_ENVIRONMENT
	// check if there is no previous photo with this name (IOS should prevent duplicate file names, so this
	// is checked only in debug mode, and should not be expected to happen in real life)
	if (g_hash_table_contains(handle->photos, photo_get_file_name(photo)))
	{
		LOG_WARN("Photo at file %s has already been added to album %s, overwriting the previous entry!",
				photo_get_file_name(photo), handle->name);
	}
#endif

	g_hash_table_insert(handle->photos, strdup(photo_get_file_name(photo)), photo);
	return true;
}

bool album_for_each_photo(const album_h handle, album_for_each_photo_cb callback, void* user_data)
{
	ASSERT_RET(handle != NULL, false);
	ASSERT_RET(callback != NULL, false);

	GHashTableIter it;
	gpointer value;

	g_hash_table_iter_init(&it, handle->photos);
	while (g_hash_table_iter_next(&it, NULL, &value))
	{
		if (!callback(handle, (photo_h) value, user_data))
		{
			break;
		}
	}

	return true;
}

photo_h album_get_photo_by_file_name(const album_h handle, const char* file_name)
{
	ASSERT_RET(handle != NULL, false);
	ASSERT_RET(file_name != NULL, false);

	photo_h photo = (photo_h) g_hash_table_lookup(handle->photos, file_name);
	if (photo == NULL)
	{
		return NULL;
	}

	return photo_ref(photo);
}

album_h album_ref(album_h handle)
{
	ASSERT_RET(handle, NULL);
	ASSERT_RET(handle->ref_count > 0, handle);

	g_atomic_int_inc(&handle->ref_count);
	return handle;
}

void album_unref(album_h handle)
{
	ASSERT_RET(handle);
	ASSERT_RET(handle->ref_count > 0);

	if (g_atomic_int_dec_and_test(&handle->ref_count))
	{
		g_hash_table_unref(handle->photos);
		free(handle->name);
		free(handle);
	}
}
