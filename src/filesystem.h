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
 * Free the previously created instance of a filesystem
 * @param handle a handle of a filesystem which should be freed
 */
void filesystem_free(filesystem_h handle);
