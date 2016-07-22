#include "filesystem.h"
#include "logger.h"
#include "device.h"
#include "db.h"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		LOG_ERROR("Usage: %s <mount location>", argv[0]);
		return 1;
	}

	device_h* devices = NULL;
	size_t devices_count = 0;

	if (get_available_devices(&devices, &devices_count))
	{
		for (size_t i = 0; i < devices_count; i++)
		{
			LOG_INFO("Found device %s (%s)", device_get_uid(devices[i]), device_get_name(devices[i]));

			char* db_location = device_get_photo_db_location(devices[i]);
			db_h db = db_create(db_location, device_get_name(devices[i]));

			if (db)
			{
				db_free(db);
			}

			free(db_location);
			device_free(devices[i]);
		}

		free(devices);
	}

	filesystem_run(argv[1]);
}
