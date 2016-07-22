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
 * @param[in] root_path an absolute path to the root directory of the corresponding
 * device
 * @return A handle for the db of the passed device
 */
db_h db_create(const char* db_location, const char* device_name, const char* root_path);

/**
 * Get the device name of the device corresponding to that db
 * @param handle a valid database handle
 * @return the device name of the corresponding device or NULL on invalid argument
 */
const char* db_get_device_name(const db_h handle);

/**
 * Get the root path of the device corresponding to that db
 * @param handle a valid database handle
 * @return the root path of the corresponding device or NULL on invalid argument
 */
const char* db_get_root_path(const db_h handle);

/**
 * Increases the reference counter of the passed db handle
 * @param handle the handle which reference counter should be increased
 * @return the handle passed as the parameter
 */
db_h db_ref(db_h handle);

/**
 * Decrease the reference counter of the passed db handle, and free
 * all associated memory if such reference counter drops to zero
 * @param[in] handle A valid db handle
 */
void db_unref(db_h handle);

