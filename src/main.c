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

			char* db_location = device_get_photo_db_location(devices[i]);
			db_h db = db_create(db_location);
			db_free(db);

			free(db_location);
			device_free(devices[i]);
		}

		free(devices);
	}
}
