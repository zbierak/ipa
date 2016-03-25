#pragma once

/**
 * A structure for storing the contents of all albums on an idevice
 */
struct db_s;

/**
 * A handle for db_s
 */
typedef struct db_s* db_h;

/**
 * Creates the instance of db for a certain idevice
 * @param[in] device A valid handle for an existing device
 * @return A handle for the db of the passed device
 */
db_h db_create(const char* db_location);

/**
 * Frees all memory of a db handle allocated with db_create()
 * @param[in] handle A valid db handle
 */
void db_free(db_h handle);
