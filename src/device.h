#pragma once

#include <stdlib.h>
#include <stdbool.h>

/**
 * A structure for storing basic information about a connected idevice.
 * For now, these include device uid and name.
 */
struct device_s;

/**
 * A handle for device_s
 */
typedef struct device_s* device_h;

/**
 * Creates the instance of device based on uid and name
 * @param[in] uid A unique uid of an idevice
 * @param[in] name A user friendly name of an idevice
 * @return A handle for the device with the passed parameters or NULL on error
 */
device_h device_create(const char* uid, const char* name);

/**
 * Gets the uid of a device
 * @param[in] handle A valid device handle
 * @return A uid of the device passed by handle or NULL when handle is NULL
 */
const char* device_get_uid(const device_h handle);

/**
 * Gets the name of a device
 * @param[in] handle A valid device handle
 * @return A name of the device passed by handle or NULL when handle is NULL
 */
const char* device_get_name(const device_h handle);


/**
 * Return the location of the photo database relative to root.
 * @param[in] device A valid handle for an existing device
 * @return A location to the photo database of the device passed as the argument
 * @note You should free the returned value by yourself
 */
char* device_get_photo_db_location(const device_h device);

/**
 * Frees all memory of a device handle allocated with device_create()
 * @param[in] handle A valid device handle
 */
void device_free(device_h handle);

/**
 * Get a list of all idevices connected to this computer
 * @param[out] devs The list of handles for connected devices
 * @param[out] devs_count length of devs array
 * @return Whether the function succeeded or failed
 * @retval true at least one connected device has been found and retrieved
 * @retval false unable to get details of at least one idevice
 * @note If retval is false, devs and devs_count are not modified
 */
bool get_available_devices(device_h** devs, size_t* devs_count);
