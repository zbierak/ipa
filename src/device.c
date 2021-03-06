#include "device.h"
#include "logger.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

#include "utils.h"

// command to retrieve the info about the device
#define IMOBILEDEVICE_CMD_DEVICE_INFO "ideviceinfo"

struct device_s
{
	char* uid;
	char* name;
};

device_h device_create(const char* uid, const char* name)
{
	device_h handle = calloc(1, sizeof(struct device_s));
	ASSERT_RET(handle != NULL, NULL);

	handle->uid = STRDUP(uid);
	handle->name = STRDUP(name);

	return handle;
}

const char* device_get_uid(const device_h handle)
{
	ASSERT_RET(handle != NULL, NULL);
	return handle->uid;
}

const char* device_get_name(const device_h handle)
{
	ASSERT_RET(handle != NULL, NULL);
	return handle->name;
}

/*
 * GVFS default location:  /run/user/$uid/gvfs/
 * iDevice under GVFS:     afc:host=$device_uid
 *
 * This function combines the two strings and returns a single address to root directory of an idevice
 *
 * @todo gvfs default location should be extracted from /etc/mtab or somewhere else
 */
char* device_get_root_path(const device_h device)
{
	const char* device_id = device_get_uid(device);
	ASSERT_RET(device_id != NULL, NULL);

	char* buffer = NULL;
	asprintf(&buffer, "/run/user/%u/gvfs/afc:host=%s/", getuid(), device_id);
	return buffer;
}

/*
 * Photo db under iDevice is located in PhotoData/Photos.sqlite
 * This function retrieves the root location of the device and adds the abovementioned path to photo db
 * to produce the absolute path to idevice database.
 */
char* device_get_photo_db_location(const device_h device)
{
	char* root_path = device_get_root_path(device);
	ASSERT_RET(root_path != NULL, NULL);

	char* buffer = NULL;
	asprintf(&buffer, "%sPhotoData/Photos.sqlite", root_path);
	free(root_path);
	return buffer;
}

void device_free(device_h handle)
{
	if (handle)
	{
		free(handle->uid);
		free(handle->name);
		free(handle);
	}
}

bool get_available_devices(device_h** devs, size_t* devs_count)
{
	ASSERT_RET(devs != NULL, false);
	ASSERT_RET(devs_count != NULL, false);

	char** devices = NULL;
	int devices_count = 0;

	if (idevice_get_device_list(&devices, &devices_count) != IDEVICE_E_SUCCESS || devices_count <= 0)
	{
		LOG_ERROR("No idevice found, please check it's plugged in...");
		return false;
	}

	size_t ldevs_count = devices_count;
	device_h* ldevs = calloc(ldevs_count, sizeof(device_h));
	ASSERT_RET(ldevs != NULL, false);

	off_t offset = 0;
	for (size_t i = 0; i < ldevs_count; i++)
	{
		idevice_t device = NULL;
		if (idevice_new(&device, devices[i]) != IDEVICE_E_SUCCESS)
		{
			LOG_ERROR("Unable to get details of device %s...", devices[i]);
			continue;
		}

		lockdownd_client_t client = NULL;
		lockdownd_error_t err = lockdownd_client_new(device, &client, IMOBILEDEVICE_CMD_DEVICE_INFO);

		if (err != LOCKDOWN_E_SUCCESS)
		{
			LOG_ERROR("Unable to perform lockdown of device %s, error code %d", devices[i], err);
			idevice_free(device);
			continue;
		}

		char* name = NULL;
		err = lockdownd_get_device_name(client, &name);
		if (name)
		{
			ldevs[offset++] = device_create(devices[i], name);
			free(name);
		}

		lockdownd_client_free(client);
		idevice_free(device);
	}

	idevice_device_list_free(devices);

	bool success = (offset > 0);
	if (success)
	{
		*devs = ldevs;
		*devs_count = ldevs_count;
	}
	else
	{
		free(ldevs);
	}

	return success;
}
