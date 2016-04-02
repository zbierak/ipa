#include "db.h"
#include "logger.h"

#include <sqlite3.h>
#include <stdlib.h>
#include <unistd.h>

struct db_s
{
	sqlite3 *db;
};

db_h db_create(const char* db_location)
{
	if (db_location == NULL)
	{
		LOG_WARN("db_location cannot be NULL");
		return NULL;
	}

	db_h handle = calloc(1, sizeof(struct db_s));

	if (handle)
	{
		LOG_DEBUG("DB path: %s", db_location);

		if (access(db_location, F_OK) == -1)
		{
			LOG_ERROR("Unable to open database %s (improper path)", db_location);
			free(handle);
			return NULL;
		}

		int rc = sqlite3_open_v2(db_location, &handle->db, SQLITE_OPEN_READONLY, NULL);

		if (rc != SQLITE_OK)
		{
			LOG_ERROR("Unable to open database %s (%s)", db_location, sqlite3_errmsg(handle->db));
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
