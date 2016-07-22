#include "db.h"
#include "album.h"
#include "utils.h"
#include "logger.h"

#include <sqlite3.h>
#include <stdlib.h>
#include <unistd.h>

#include <glib.h>

// the name of the table containing the photos
#define PHOTO_TABLE_NAME 	"ZGENERICASSET"

// the name of the table containing the photo albums
#define ALBUM_TABLE_NAME 	"ZGENERICALBUM"

/**
 * A structure behind db_h handle
 */
typedef struct db_s
{
	sqlite3* db;                    /// the active connection to the sqlite database
	char* device_name;              /// the human-readable of the corresponding device (may not be globally unique)

	char* assets_table_name;        /// discovered table name storing assets (see verify_database_sanity())
	char* assets_album_fk;          /// discovered foreign key of album in assets table (see verify_database_sanity())
	char* assets_photo_fk;          /// discovered foreign key of photo in assets table (see verify_database_sanity())

	GHashTable* albums;             /// lookup table of all albums retrieved from database <album-name, album details> [char*, album_h]
} db_t;

static bool verify_database_sanity(db_h handle)
{
	/*
	 *  The name of the ASSETS table seems to change between the devices (Z_?ASSETS, where ? is an
	 *  arbitrary (?) number). They seem to be somehow related to the contents of the Z_PRIMARYKEY
	 *  table, but I'm not entirely sure this is the right place to look.
	 *
	 *  In here we take a simpler path and extract the name of the assets table from the list of
	 *  all tables in the sqlite database (since it always ends with an ASSETS postfix)
	 */
	static const char* table_query = "select name from sqlite_master where type='table' and name like '%ASSETS'";

	int table_query_callback(void* user_data, int col_count, char** record, char** col_names)
	{
		db_h handle = (db_h) user_data;

		if (col_count != 1)
		{
			LOG_WARN("The query should return only one column");
			return -1;
		}

		if (handle->assets_table_name != NULL)
		{
			LOG_WARN("Multiple assets tables found. This case is not yet supported.");
			return -1;
		}

		handle->assets_table_name = STRDUP(record[0]);
		return 0;
	}

	sqlite3_exec(handle->db, table_query, table_query_callback, handle, NULL);

	if (handle->assets_table_name == NULL)
	{
		LOG_WARN("No asset table found, unable to proceed.");
		return false;
	}

	LOG_DEBUG("Assets table name for device %s is %s", handle->device_name, handle->assets_table_name);

	/*
	 * Wait for it, there is more. It seems that the column names for the assets table can also change.
	 * So we need to determine them somehow. Below we just query the
	 */
	char* column_query = NULL;
	asprintf(&column_query, "pragma table_info('%s')", handle->assets_table_name);

	int column_query_callback(void* user_data, int col_count, char** record, char** col_names)
	{
		db_h handle = (db_h) user_data;

		if (col_count < 2)
		{
			LOG_WARN("The query should return at least two columns");
			return -1;
		}

		/*
		 * Extract column names for album and asset (photo). Note: there seem to be two
		 * column names ending with "ASSETS", we want that without a "FOK" infix (whatever
		 * that is)
		 */
		char* column_name = record[1];
		if (STRSTR(column_name, "ALBUMS") && (handle->assets_album_fk == NULL))
		{
			handle->assets_album_fk = STRDUP(column_name);
		}
		else if (STRSTR(column_name, "ASSETS") &&
				(!STRSTR(column_name, "FOK")) &&
				(handle->assets_photo_fk == NULL))
		{
			handle->assets_photo_fk = STRDUP(column_name);
		}

		return 0;
	}

	sqlite3_exec(handle->db, column_query, column_query_callback, handle, NULL);
	free(column_query);

	LOG_DEBUG("Assets album fk: %s", handle->assets_album_fk);
	LOG_DEBUG("Assets photo fk: %s", handle->assets_photo_fk);

	return handle->assets_album_fk != NULL && handle->assets_photo_fk != NULL;
}

static bool db_extract_photos(db_h handle)
{
	ASSERT_RET(handle != NULL, false);

	/*
	 * In here we extract all photos assigned to each user-created album. We know an album
	 * has been created by the user if it has ZKIND = 2 (magic numbers, yay!)
	 */

	char* query = NULL;
	asprintf(&query, "select %s.ZFILENAME, %s.ZDIRECTORY, %s.ZTITLE "
		"from %s "
		"inner join %s on %s.Z_PK = %s.%s "
		"inner join %s on %s.%s = %s.Z_PK "
		"where %s.ZKIND = 2;",
		PHOTO_TABLE_NAME, PHOTO_TABLE_NAME, ALBUM_TABLE_NAME,
		PHOTO_TABLE_NAME,
		handle->assets_table_name, PHOTO_TABLE_NAME, handle->assets_table_name, handle->assets_photo_fk,
		ALBUM_TABLE_NAME, handle->assets_table_name, handle->assets_album_fk, ALBUM_TABLE_NAME,
		ALBUM_TABLE_NAME);

	LOG_DEBUG("Photo query: %s", query);

	int query_callback(void* user_data, int col_count, char** record, char** col_names)
	{
		db_h handle = (db_h) user_data;
		ASSERT_RET(handle != NULL, -1);

		if (col_count != 3)
		{
			LOG_WARN("The query should return exactly three columns");
			return -1;
		}

		const char* file_name = record[0];
		const char* location = record[1];
		const char* album_name = record[2];

		album_h album = g_hash_table_lookup(handle->albums, album_name);
		if (album == NULL)
		{
			album = album_create(album_name);
			g_hash_table_insert(handle->albums, strdup(album_name), album);
		}

		album_add_photo(album, photo_create(file_name, location));

		return 0;
	}

	sqlite3_exec(handle->db, query, query_callback, handle, NULL);
	free(query);

	return true;
}

db_h db_create(const char* db_location, const char* device_name)
{
	ASSERT_RET(db_location != NULL, NULL);
	ASSERT_RET(device_name != NULL, NULL);

	db_h handle = calloc(1, sizeof(struct db_s));

	if (handle)
	{
		LOG_DEBUG("DB path for device %s: %s", device_name, db_location);
		handle->device_name = strdup(device_name);
		handle->albums = g_hash_table_new_full(g_str_hash, g_str_equal, free, (GDestroyNotify) album_unref);

		if (access(db_location, F_OK) == -1)
		{
			LOG_ERROR("Unable to open database %s (improper path)", db_location);
			db_free(handle);
			return NULL;
		}

		int rc = sqlite3_open_v2(db_location, &handle->db, SQLITE_OPEN_READONLY, NULL);

		if (rc != SQLITE_OK)
		{
			LOG_ERROR("Unable to open database of device %s (%s)", device_name, sqlite3_errmsg(handle->db));
			db_free(handle);
			return NULL;
		}

		if (!verify_database_sanity(handle))
		{
			LOG_ERROR("Malformed photo database of device %s", device_name);
			db_free(handle);
			return NULL;
		}
	}

	if (!db_extract_photos(handle))
	{
		LOG_ERROR("Unable to perform an initial photo extraction of device %s", device_name);
		db_free(handle);
		return NULL;
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

		g_hash_table_unref(handle->albums);
		free(handle->device_name);
		free(handle->assets_table_name);
		free(handle->assets_album_fk);
		free(handle->assets_photo_fk);
		free(handle);
	}
}


