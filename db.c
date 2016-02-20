#include "db.h"

#include <sqlite3.h>
#include <unistd.h>
#include <stdio.h>

struct db_s
{
	sqlite3 *db;
};

/**
 * Return the location of the photo database relative to root.
 *
 * GVFS default location:  /run/user/$uid/gvfs/
 * iDevice under GVFS:     afc:host=$device_uid
 * Photo db under iDevice: PhotoData/Photos.sqlite
 *
 * This function combines the three strings and returns a single address to idevice database
 *
 * @param[in] device A valid handle for an existing device
 * @return A location to the photo database of the device passed as the argument
 * @note You should free the returned value by yourself
 *
 * @todo gvfs default location should be extracted from /etc/mtab or somewhere else
 */
static char* get_photo_database_location(const device_h device)
{
	const char* device_id = device_get_uid(device);
	if (device_id == NULL)
	{
		return NULL;
	}

	char* buffer = NULL;
	asprintf(&buffer, "/run/user/%u/gvfs/afc:host=%s/PhotoData/Photos.sqlite", getuid(), device_id);
	return buffer;
}

db_h db_create(const device_h device)
{
	db_h handle = calloc(1, sizeof(struct db_s));

	if (handle)
	{
		char* location = get_photo_database_location(device);
		printf("DB path: %s\n", location);

		if (access(location, F_OK) == -1)
		{
			fprintf(stderr, "Unable to open database on device %s (improper path)\n", device_get_uid(device));
			free(location);
			free(handle);
			return NULL;
		}

		int rc = sqlite3_open_v2(location, &handle->db, SQLITE_OPEN_READONLY, NULL);
		free(location);

		if (rc != SQLITE_OK)
		{
			fprintf(stderr, "Unable to open database on device %s (%s)\n", device_get_uid(device), sqlite3_errmsg(handle->db));
			sqlite3_close(handle->db);
			free(handle);
			return NULL;
		}
	}

	return handle;
}


void db_free(db_h handle)
{
	if (handle)
	{
		if (handle->db)
		{
			sqlite3_close(handle->db);
		}

		free(handle);
	}
}