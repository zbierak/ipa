#pragma once

/**
 * A handle to a structure representing a single photo
 */
typedef struct photo_s* photo_h;

/**
 * Create a new instance of a photo with the provided parameters
 * @param file_name the file name of a photo, without a path
 * @param location the path where the photo is located, relative to
 * the root directory of its device.
 * @return a new instance of a photo structure or NULL on error
 */
photo_h photo_create(const char* file_name, const char* location);

/**
 * Get the file name of the passed photo
 * @param handle a valid handle to a photo structure
 * @return the file name from the passed photo structure or NULL on error
 */
const char* photo_get_file_name(const photo_h handle);

/**
 * Get the location of the passed photo
 * @param handle a valid handle to a photo structure
 * @return the location from the passed photo structure or NULL on error
 */
const char* photo_get_location(const photo_h handle);

/**
 * Increase the reference counter of the passed photo handle
 * @param handle a valid handle to a photo structure
 * @return the photo handle passed as the parameter to this function
 */
photo_h photo_ref(photo_h handle);

/**
 * Decrease the reference counter to the passed photo handle, and free
 * all connected memory if the reference counter drops to zero
 * @param handle a valid handle to a photo structure
 */
void photo_unref(photo_h handle);
