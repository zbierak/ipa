#include <stdio.h>

#include "device.h"
#include "db.h"

int main()
{
	device_h* devices = NULL;
	size_t devices_count = 0;

	if (get_available_devices(&devices, &devices_count))
	{
		for (size_t i = 0; i < devices_count; i++)
		{
			printf("Found device %s (%s)\n", device_get_uid(devices[i]), device_get_name(devices[i]));

			db_h db = db_create(devices[i]);
			db_free(db);

			device_free(devices[i]);
		}

		free(devices);
	}
}
