#include "device.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

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

	if (handle)
	{
		handle->uid = STRDUP(uid);
		handle->name = STRDUP(name);
	}

	return handle;
}

const char* device_get_uid(const device_h handle)
{
	assert(handle);
	return handle->uid;
}

const char* device_get_name(const device_h handle)
{
	assert(handle);
	return handle->name;
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
	if (devs == NULL || devs_count == NULL)
	{
		return false;
	}

	char** devices = NULL;
	int devices_count = 0;

	if (idevice_get_device_list(&devices, &devices_count) != IDEVICE_E_SUCCESS || devices_count <= 0)
	{
		fprintf(stderr, "No idevice found, please check it's plugged in...\n");
		return false;
	}

	size_t ldevs_count = devices_count;
	device_h* ldevs = calloc(ldevs_count, sizeof(device_h));

	off_t offset = 0;
	for (size_t i = 0; i < ldevs_count; i++)
	{
		idevice_t device = NULL;
		if (idevice_new(&device, devices[i]) != IDEVICE_E_SUCCESS)
		{
			printf("Warning: unable to get details of device %s...\n", devices[i]);
			continue;
		}

		lockdownd_client_t client = NULL;
		lockdownd_error_t err = lockdownd_client_new(device, &client, IMOBILEDEVICE_CMD_DEVICE_INFO);

		if (err != LOCKDOWN_E_SUCCESS)
		{
			printf("Warning: unable to perform lockdown of device %s, error code %d\n", devices[i], err);
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