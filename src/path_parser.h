/*
 * This module is responsible for parsing the path retrieved from fuse and performing
 * the specified events depending on the type of path. Currently, the fuse path is
 * in the following format "/[device[/album[/photo]]]", where each bracket contains
 * optional data (for instance since the filesystem user performs ls within the devices
 * directory). As such, the path parser traverses the obtained path and based on the
 * data from the program database (from filesystem.h) it retrieves the parameters of
 * the deepest element within the path and retrieves a respective callback (depending
 * whether it is a root element, device, album or photo).
 */

#pragma once

#include <stdbool.h>

#include "db.h"
#include "album.h"
#include "photo.h"
#include "filesystem.h"

/**
 * Callback invoked whenever the deepest path element passed to path parser is root ('/')
 * @param user_data the user data passed to path_parser_execute()
 */
typedef void (*on_root_cb)(void* user_data);

/**
 * Callback invoked whenever the deepest path element passed to path parser is a device name
 * @param device_db the database of the device corresponding to the device from the path
 * @param user_data the user data passed to path_parser_execute()
 */
typedef void (*on_device_cb)(const db_h device_db, void* user_data);

/**
 * Callback invoked whenever the deepest path element passed to path parser is an album name
 * @param device_db the database of the device to which the album belongs
 * @param album the album identified by the name retrieved from the path
 * @param user_data the user data passed to path_parser_execute()
 */
typedef void (*on_album_cb)(const db_h device_db, const album_h album, void* user_data);

/**
 * Callback invoked whenever the deepest path element passed to path parser is a photo
 * @param device_db the database of the device to which the album belongs
 * @param album the album to which the retrieved photo belongs
 * @param photo the photo identified by the name retrieved from the path
 * @param user_data the user data passed to path_parser_execute()
 */
typedef void (*on_photo_cb)(const db_h device_db, const album_h album, const photo_h photo, void* user_data);

/**
 * Structure holding callbacks passed to path_parser_execute()
 */
typedef struct path_parser_cb_s
{
	on_root_cb on_root;
	on_device_cb on_device;
	on_album_cb on_album;
	on_photo_cb on_photo;

	void* on_root_user_data;
	void* on_device_user_data;
	void* on_album_user_data;
	void* on_photo_user_data;
} path_parser_cb_t;

/**
 * A handle of a path parser
 */
typedef struct path_parser_s* path_parser_h;

/**
 * Creates a new instance of path parser assigned to a certain filesystem.
 * @return a new instance of path parser or NULL on error
 * @note the passed filesystem pointer must be valid at least until path_parser_free() is called
 * on the handle returned by this function.
 */
path_parser_h path_parser_create(filesystem_h fs);

/**
 * Parses the path retrieved from FUSE and invokes the corresponding callbacks based on what the
 * deepest element of the path contains.
 * @param path the path retrieved from FUSE, currently in format '/[device[/album[/photo]]]', where
 * all within the square brackets might be optional
 * @param fs a valid handle of a filesystem
 * @param callbacks callbacks which should be invoked while parsing
 * @return true if a root elemet/device/album/photo has been found within the path or false if the
 * path refers to an object which has not been found in the passed filesystem element.
 */
bool path_parser_execute(path_parser_h handle, const char* path, path_parser_cb_t callbacks);

/**
 * Frees all memory assigned with an instance of path parser
 * @param handle a handle to a path parser which should be freed
 */
void path_parser_free(path_parser_h handle);
