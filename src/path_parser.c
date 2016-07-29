#include "path_parser.h"

#include "logger.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glib.h>

// uncomment this if you want to be notified if the path could not be parsed into an element existing in db
//#define WARN_ABOUT_FAILED_TRANSLATION

/**
 * A structure behind path_parser_h handle
 */
struct path_parser_s
{
	filesystem_h fs;
	GHashTable* cache;
};

// the maximum number of entities (devices, albums, photos) stored within the lookup cache at any single point in time
#define MAX_CACHE_SIZE 10000

// elements of path parser cache, enabling for fast lookup of paths instead of manually parsing them everything.

/**
 * A type of an element stored in cache
 */
typedef enum
{
	PPCE_DEVICE = 1,//!< it's a device
	PPCE_ALBUM,     //!< it's an album
	PPCE_PHOTO      //!< it's a photo
} pp_cache_elem_type_e;

/**
 * A single entry stored within a cache, either a device, an album or a photo
 */
typedef struct pp_cache_elem_s
{
	pp_cache_elem_type_e type;
	db_h device;
	album_h album;
	photo_h photo;
}* pp_cache_elem_h;

static pp_cache_elem_h ppce_create_from_device(const db_h device)
{
	pp_cache_elem_h handle = (pp_cache_elem_h) calloc(1, sizeof(struct pp_cache_elem_s));
	ASSERT_RET(handle != NULL, NULL);

	handle->type = PPCE_DEVICE;
	handle->device = db_ref(device);

	return handle;
}

static pp_cache_elem_h ppce_create_from_album(const db_h device, const album_h album)
{
	pp_cache_elem_h handle = (pp_cache_elem_h) calloc(1, sizeof(struct pp_cache_elem_s));
	ASSERT_RET(handle != NULL, NULL);

	handle->type = PPCE_ALBUM;
	handle->device = db_ref(device);
	handle->album = album_ref(album);

	return handle;
}

static pp_cache_elem_h ppce_create_from_photo(const db_h device, const album_h album, const photo_h photo)
{
	pp_cache_elem_h handle = (pp_cache_elem_h) calloc(1, sizeof(struct pp_cache_elem_s));
	ASSERT_RET(handle != NULL, NULL);

	handle->type = PPCE_PHOTO;
	handle->device = db_ref(device);
	handle->album = album_ref(album);
	handle->photo = photo_ref(photo);

	return handle;
}

static void ppce_free(pp_cache_elem_h handle)
{
	if (handle)
	{
		if (handle->device)
		{
			db_unref(handle->device);
		}

		if (handle->album)
		{
			album_unref(handle->album);
		}

		if (handle->photo)
		{
			photo_unref(handle->photo);
		}

		free(handle);
	}
}

static pp_cache_elem_h pp_cache_lookup(path_parser_h handle, const char* path)
{
	return g_hash_table_lookup(handle->cache, path);
}

static void pp_cache_insert(path_parser_h handle, const char* path, pp_cache_elem_h elem)
{
	ASSERT_RET(handle);
	ASSERT_RET(path);
	ASSERT_RET(elem);

	if (g_hash_table_size(handle->cache) >= MAX_CACHE_SIZE)
	{
		/*
		 * Remove one element at random from the cache. This might not be
		 * the most sophisticated cache invalidation method, but is relatively
		 * simple to implement and does not introduce lots of overhead, assuming
		 * that iterating over MAX_CACHE_SIZE elements is still faster that
		 * parsing the path from scratch. Well, in any case this seems to be
		 * working just fine for now, if some performance issues occur, it might
		 * be possible to consider a rework of this approach.
		 */
		size_t target = rand() % g_hash_table_size(handle->cache);
		size_t pos = 0;

		GHashTableIter it;
		gpointer key;

		g_hash_table_iter_init(&it, handle->cache);
		while (g_hash_table_iter_next(&it, &key, NULL))
		{
			if (target == pos++)
			{
				g_hash_table_remove(handle->cache, key);
				break;
			}
		}
	}

	g_hash_table_insert(handle->cache, strdup(path), elem);
}

// the actual part of path parser, which retrieves the information either from cache, or the path itself

path_parser_h path_parser_create(filesystem_h fs)
{
	path_parser_h handle = (path_parser_h) calloc(1, sizeof(struct path_parser_s));
	ASSERT_RET(handle != NULL, NULL);

	handle->fs = fs;
	handle->cache = g_hash_table_new_full(g_str_hash, g_str_equal, free, (GDestroyNotify) ppce_free);

	srand(time(NULL));

	return handle;
}

static bool process_photo(path_parser_h handle, const char* original_path, char* relative_path, db_h db, album_h album, path_parser_cb_t callbacks)
{
	photo_h photo = album_get_photo_by_file_name(album, relative_path);
	if (photo == NULL)
	{
		#ifdef WARN_ABOUT_FAILED_TRANSLATION
		LOG_WARN("Unable to retrieve photo with name '%s' from album '%s'", relative_path, album_get_name(album));
		#endif

		return false;
	}

	if (callbacks.on_photo)
	{
		callbacks.on_photo(db, album, photo, callbacks.on_photo_user_data);
	}

	pp_cache_insert(handle, original_path, ppce_create_from_photo(db, album, photo));

	photo_unref(photo);
	return true;
}

static bool process_album(path_parser_h handle, const char* original_path, char* processing_path, db_h db, path_parser_cb_t callbacks)
{
	char* album = processing_path;
	char* next = strchr(processing_path, '/');

	if (next != NULL)
	{
		*next = 0;
		next++;
	}

	album_h am = db_get_album_by_name(db, album);
	if (am == NULL)
	{
		#ifdef WARN_ABOUT_FAILED_TRANSLATION
		LOG_WARN("Unable to retrieve album with name '%s'", album);
		#endif

		return false;
	}

	bool success = true;
	if (next == NULL)
	{
		// this is the last component in the path, invoke callback
		if (callbacks.on_album)
		{
			callbacks.on_album(db, am, callbacks.on_album_user_data);
		}

		pp_cache_insert(handle, original_path, ppce_create_from_album(db, am));
	}
	else
	{
		// more components on the way, proceed with parsing
		success = process_photo(handle, original_path, next, db, am, callbacks);
	}

	album_unref(am);
	return success;
}

static bool process_device(path_parser_h handle, const char* original_path, char* relative_path, filesystem_h fs, path_parser_cb_t callbacks)
{
	char* device = relative_path;
	char* next = strchr(relative_path, '/');

	if (next != NULL)
	{
		*next = 0;
		next++;
	}

	db_h db = filesystem_get_database_by_fs_name(fs, device);
	if (db == NULL)
	{
		#ifdef WARN_ABOUT_FAILED_TRANSLATION
		LOG_WARN("Unable to retrieve device with name '%s'", device);
		#endif

		return false;
	}

	bool success = true;
	if (next == NULL)
	{
		// this is the last component in the path, invoke callback
		if (callbacks.on_device)
		{
			callbacks.on_device(db, callbacks.on_device_user_data);
		}

		pp_cache_insert(handle, original_path, ppce_create_from_device(db));
	}
	else
	{
		// more components on the way, proceed with parsing
		success = process_album(handle, original_path, next, db, callbacks);
	}

	db_unref(db);
	return success;
}

bool path_parser_execute(path_parser_h handle, const char* path, path_parser_cb_t callbacks)
{
	ASSERT_RET(fs != NULL, false);
	ASSERT_RET(path != NULL, false);
	ASSERT_RET(path[0] != '/', false);

	if (path[1] == 0)
	{
		if (callbacks.on_root)
		{
			callbacks.on_root(callbacks.on_root_user_data);
		}
		return true;
	}

	pp_cache_elem_h ce = pp_cache_lookup(handle, path);
	if (ce != NULL)
	{
		switch (ce->type)
		{
		case PPCE_DEVICE:
			if (callbacks.on_device)
			{
				callbacks.on_device(ce->device, callbacks.on_device_user_data);
			}
			break;
		case PPCE_ALBUM:
			if (callbacks.on_album)
			{
				callbacks.on_album(ce->device, ce->album, callbacks.on_album_user_data);
			}
			break;
		case PPCE_PHOTO:
			if (callbacks.on_photo)
			{
				callbacks.on_photo(ce->device, ce->album, ce->photo, callbacks.on_photo_user_data);
			}
			break;
		}
		return true;
	}

	char* p = strdup(path + 1);
	bool success = process_device(handle, path, p, handle->fs, callbacks);
	free(p);
	return success;
}

void path_parser_free(path_parser_h handle)
{
	if (handle)
	{
		g_hash_table_unref(handle->cache);
		free(handle);
	}
}
