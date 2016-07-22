#pragma once

#include "photo.h"

#include <stdbool.h>

/**
 * A handle holding information about an album containing photos
 */
typedef struct album_s* album_h;

/**
 * A callback invoked by album_for_each_photo() for each photo belonging to an album
 * @param handle a handle of an album to which the photo belongs
 * @param photo the photo which belongs to an album
 * @param user_data user data passed to album_for_each_photo()
 * @return true if you want to continue invoking this callback for subsequent photos, or false if
 * you don't care about the remaining photos and album_for_each_photo() should be immediately terminated.
 */
typedef bool (*album_for_each_photo_cb)(const album_h handle, const photo_h photo, void* user_data);

/**
 * Create a new instance of an album
 * @param name the name of an album
 * @return a valid handle to the newly created album or NULL on error
 */
album_h album_create(const char* name);

/**
 * Get the name of an album
 * @param handle a valid album handle
 * @return the name of an album or NULL on error
 */
const char* album_get_name(const album_h handle);

/**
 * Adds a photo to the album
 * @param handle a handle of an album to which a photo should be added
 * @param photo a photo which should be added to the album.
 * @return true when a photo was successfully added, false on error
 * @warning this function takes ownership of photo parameter, if you need yourself, you should create
 * a separate reference using photo_ref() and unreference it when you no longer need it.
 */
bool album_add_photo(album_h handle, photo_h photo);

/**
 * This function synchronously calls the passed callback for each photo from the provided album
 * @param handle the handle of an album for which the photos should be reported
 * @param callback the callback which should be invoked for each photo from an album
 * @param user_data the user data which should be passed to the callback
 * @return true on success, false if the provided arguments were incorrect
 */
bool album_for_each_photo(const album_h handle, album_for_each_photo_cb callback, void* user_data);

/**
 * Get the photo from an album by the photo's file name
 * @param handle a valid handle to the album which should be searched for the photo
 * @param file_name a file name of a photo which should be retrieved from an album
 * @return a handle to a photo identified by the provided file name or NULL when no
 * such photo exists in the passed album
 * @note you should unreference the returned value (whenever it's non-null) using
 * photo_unref() function, when you no longer need it
 */
photo_h album_get_photo_by_file_name(const album_h handle, const char* file_name);

/**
 * Increase the reference counter of the passed album
 * @param handle a handle which reference counter should be increased
 * @return the handle passed as the parameter
 */
album_h album_ref(album_h handle);

/**
 * Decrease the reference counter of the passed album, and free all the memory associated
 * with that album if the counter drops to zero
 * @param handle a handle which reference counter should be decreased
 */
void album_unref(album_h handle);
