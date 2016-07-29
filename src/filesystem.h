#pragma once

#include "db.h"

#include <stdbool.h>

/**
 * A handle representing an instance of a filesystem
 */
typedef struct filesystem_s* filesystem_h;

/**
 * Create a new instance of the filesystem
 * @return a handle to the newly created instance or NULL on error
 */
filesystem_h filesystem_create(void);

/**
 * Run the created filesystem. This function blocks until otherwise interrupted (e.g. ^+C)
 * @param handle a valid handle of a previously created filesystem
 * @param location The location where the filesystem should be mounted
 * @note in current implementation this function must be called at most once
 * at a time. In other words, only a single filesystem might be simultaneously
 * running. This simplifies the communication with the fuse library, and is possible
 * since right now only a single filesystem is expected to be running at any
 * given moment in time.
 */
void filesystem_run(filesystem_h handle, const char* location);

/**
 * Add a database of a device to the filesystem
 * @param handle a valid handle of a previously created filesystem
 * @param database the photo database of a device which should become visible within that filesystem
 * @return true if the database was successfully added or false on error
 * @warning this function takes ownership of database parameter, if you need yourself, you should create
 * a separate reference using db_ref() and unreference it when you no longer need it.
 */
bool filesystem_add_database(filesystem_h handle, db_h database);

/**
 * Retrieve a database of an existing device by its filesystem name, that is the unique name
 * assigned by the filesystem module.
 *
 * The filesystem name is a unique name of a device which is presented to FUSE. On more details
 * how this filename is created, see the implementation of filesystem_add_database(), although
 * in practice this knowledge should not be required.
 * @param handle a valid handle of a previously created filesystem
 * @param fs_name the filesystem name of a device retrieved from FUSE
 * @return a handle of a device database if one identified by fs_name has been found, or NULL when
 * no device with such filesystem name currently exists. Whenever the result value is not NULL, you
 * should manually unreference it by calling db_unref() when you no longer need it.
 */
db_h filesystem_get_database_by_fs_name(filesystem_h handle, const char* fs_name);

/**
 * Free the previously created instance of a filesystem
 * @param handle a handle of a filesystem which should be freed
 */
void filesystem_free(filesystem_h handle);
