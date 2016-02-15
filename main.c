#include <stdio.h>
#include <unistd.h>

#include "device.h"

int main()
{
	device_h* devices = NULL;
	size_t devices_count = 0;

	if (get_available_devices(&devices, &devices_count))
	{
		for (size_t i = 0; i < devices_count; i++)
		{
			printf("Found device %s (%s)\n", device_get_uid(devices[i]), device_get_name(devices[i]));
			device_free(devices[i]);
		}

		free(devices);
	}

	// TODO: will access /run/user/$uid/gvfs/...
	uid_t uid = getuid();
	printf("Hello user %d!\n", uid);
}
