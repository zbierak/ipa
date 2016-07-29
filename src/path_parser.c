#include "path_parser.h"

#include "logger.h"
#include "utils.h"

#include <string.h>

static bool process_photo(char* path, db_h db, album_h album, path_parser_cb_t callbacks)
{
	photo_h photo = album_get_photo_by_file_name(album, path);
	if (photo == NULL)
	{
		LOG_WARN("Unable to retrieve photo with name '%s' from album '%s'", path, album_get_name(album));
		return false;
	}

	if (callbacks.on_photo)
	{
		callbacks.on_photo(db, album, photo, callbacks.on_photo_user_data);
	}

	photo_unref(photo);
	return true;
}

static bool process_album(char* path, db_h db, path_parser_cb_t callbacks)
{
	char* album = path;
	char* next = strchr(path, '/');

	if (next != NULL)
	{
		*next = 0;
		next++;
	}

	album_h am = db_get_album_by_name(db, album);
	if (am == NULL)
	{
		LOG_WARN("Unable to retrieve album with name '%s'", album);
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
	}
	else
	{
		// more components on the way, proceed with parsing
		success = process_photo(next, db, am, callbacks);
	}

	album_unref(am);
	return success;
}

static bool process_device(char* path, filesystem_h fs, path_parser_cb_t callbacks)
{
	char* device = path;
	char* next = strchr(path, '/');

	if (next != NULL)
	{
		*next = 0;
		next++;
	}

	db_h db = filesystem_get_database_by_fs_name(fs, device);
	if (db == NULL)
	{
		LOG_WARN("Unable to retrieve device with name '%s'", device);
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
	}
	else
	{
		// more components on the way, proceed with parsing
		success = process_album(next, db, callbacks);
	}

	db_unref(db);
	return success;
}

bool path_parser_execute(const char* path, filesystem_h fs, path_parser_cb_t callbacks)
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

	char* p = strdup(path + 1);
	bool success = process_device(p, fs, callbacks);
	free(p);
	return success;
}
