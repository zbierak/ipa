#pragma once

#include <stdbool.h>

#include "album.h"

/**
 * A structure for storing the contents of all albums on an idevice
 */
struct db_s;

/**
 * A handle for db_s
 */
typedef struct db_s* db_h;

/**
 * A callback invoked by db_for_each_album() for each album belonging to a device database
 * @param handle a handle of a device database to which the album belongs
 * @param album the album which belongs to the device database
 * @param user_data user data passed to db_for_each_album()
 * @return true if you want to continue invoking this callback for subsequent albums, or false if
 * you don't care about the remaining albums and db_for_each_album() should be immediately terminated.
 */
typedef bool (*db_for_each_album_cb)(const db_h handle, const album_h album, void* user_data);

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
 * This function synchronously calls the passed callback for each album from the provided device database
 * @param handle the handle of a device database for which the albums should be reported
 * @param callback the callback which should be invoked for each album from a device database
 * @param user_data the user data which should be passed to the callback
 * @return true on success, false if the provided arguments were incorrect
 */
bool db_for_each_album(const db_h handle, db_for_each_album_cb callback, void* user_data);

/**
 * Get album from the database by its name
 * @param handle a valid database handle
 * @param album_name the name of the album which should be retrieved
 * @return the handle of the album with the provided album_name or NULL when no such
 * album has been found. If the return value is not NULL, you should unreference it
 * with album_unref() when you no longer need it.
 */
album_h db_get_album_by_name(const db_h handle, const char* album_name);

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

