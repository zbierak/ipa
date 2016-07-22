#pragma once

#include <stdbool.h>

/**
 * A structure for storing the contents of all albums on an idevice
 */
struct db_s;

/**
 * A handle for db_s
 */
typedef struct db_s* db_h;

/**
 * Creates the instance of db for the specified location
 * @param[in] db_location a location of the database which should be opened
 * @param[in] device_name a name of the device corresponding to the database
 * passed in db_location (performs purely informative function and is only used
 * in messages passed to the user, instead of db_location)
 * @return A handle for the db of the passed device
 */
db_h db_create(const char* db_location, const char* device_name);

/**
 * Frees all memory of a db handle allocated with db_create()
 * @param[in] handle A valid db handle
 */
void db_free(db_h handle);

