#include "filesystem.h"

#include "path_parser.h"
#include "logger.h"
#include "utils.h"
#include "db.h"

#define FUSE_USE_VERSION 29

#include <fuse.h>
#include <glib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>

typedef struct filesystem_s
{
	GHashTable* devices;         /// lookup table for databases of devices <unique-device-name,database details> [char*,db_h]
} filesystem_t;

/**
 * The currently running filesystem instance. This value is stored to be reutilized by fs_* events
 * from fuse. This can be done like that, since we assume that at most one filesystem is running
 * at any moment in time.
 */
static filesystem_h fs_instance = NULL;

#define DEFAULT_MODE_DIRECTORY S_IFDIR | S_IRUSR | S_IXUSR
#define DEFAULT_MODE_PHOTO S_IFREG | S_IRUSR

static void getaatr_root(void* user_data)
{
	struct stat* stbuf = (struct stat*) user_data;
	stbuf->st_mode = DEFAULT_MODE_DIRECTORY;
	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
}

static void getaatr_device(const db_h db, void* user_data)
{
	struct stat* stbuf = (struct stat*) user_data;
	lstat(db_get_root_path(db), stbuf);
	stbuf->st_mode = DEFAULT_MODE_DIRECTORY;
}

static void getattr_album(const db_h db, const album_h album, void* user_data)
{
	struct stat* stbuf = (struct stat*) user_data;
	lstat(db_get_root_path(db), stbuf);
	stbuf->st_mode = DEFAULT_MODE_DIRECTORY;
}

static void getattr_photo(const db_h device_db, const album_h album, const photo_h photo, void* user_data)
{
	struct stat* stbuf = (struct stat*) user_data;
	lstat(photo_get_location(photo), stbuf);
	stbuf->st_mode = DEFAULT_MODE_PHOTO;
}

static int fs_getattr(const char* path, struct stat* stbuf)
{
	ASSERT_RET(fs_instance != NULL, -ENOENT);
	ASSERT_RET(path != NULL, -ENOENT);

	if (path_parser_execute(path, fs_instance, (path_parser_cb_t) {
		.on_root = getaatr_root,
		.on_device = getaatr_device,
		.on_album = getattr_album,
		.on_photo = getattr_photo,
		.on_root_user_data = stbuf,
		.on_device_user_data = stbuf,
		.on_album_user_data = stbuf,
		.on_photo_user_data = stbuf
	}))
	{
		return 0;
	}

	return -ENOENT;
}

typedef struct
{
	void* buf;
	fuse_fill_dir_t filler;
} fuse_readdir_params_t;

static void readdir_root(void* user_data)
{
	fuse_readdir_params_t* params = (fuse_readdir_params_t*) user_data;

	GHashTableIter it;
	gpointer key;

	g_hash_table_iter_init(&it, fs_instance->devices);
	while (g_hash_table_iter_next(&it, &key, NULL))
	{
		params->filler(params->buf, (const char*) key, NULL, 0);
	}
}

static bool readdir_device_for_each_album(const db_h handle, const album_h album, void* user_data)
{
	fuse_readdir_params_t* params = (fuse_readdir_params_t*) user_data;
	params->filler(params->buf, album_get_name(album), NULL, 0);
	return true;
}

static void readdir_device(const db_h db, void* user_data)
{
	db_for_each_album(db, readdir_device_for_each_album, user_data);
}

static bool readdir_album_for_each_photo(const album_h handle, const photo_h photo, void* user_data)
{
	fuse_readdir_params_t* params = (fuse_readdir_params_t*) user_data;
	params->filler(params->buf, photo_get_file_name(photo), NULL, 0);
	return true;
}

static void readdir_album(const db_h db, const album_h album, void* user_data)
{
	album_for_each_photo(album, readdir_album_for_each_photo, user_data);
}

static int fs_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info* fi)
{
	ASSERT_RET(fs_instance != NULL, -ENOENT);
	ASSERT_RET(path != NULL, -ENOENT);

	fuse_readdir_params_t params = {
		.buf = buf,
		.filler = filler
	};

	if (path_parser_execute(path, fs_instance, (path_parser_cb_t) {
		.on_root = readdir_root,
		.on_device = readdir_device,
		.on_album = readdir_album,
		.on_root_user_data = &params,
		.on_device_user_data = &params,
		.on_album_user_data = &params
	}))
	{
		return 0;
	}

	return -ENOENT;
}

static void open_photo(const db_h device_db, const album_h album, const photo_h photo, void* user_data)
{
	struct fuse_file_info* fi = (struct fuse_file_info*) user_data;

	int fd = open(photo_get_location(photo), fi->flags);

	if (fd != -1)
	{
		fi->fh = fd;
	}
}

static int fs_open(const char* path, struct fuse_file_info* fi)
{
	fi->fh = -1;

	path_parser_execute(path, fs_instance, (path_parser_cb_t) {
		.on_photo = open_photo,
		.on_photo_user_data = fi
	});

	if (fi->fh != -1)
	{
		return 0;
	}

	return -ENOENT;
}

static int fs_read(const char* path, char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi)
{
	int result = pread(fi->fh, buf, size, offset);
	if (result == -1)
	{
		return -errno;
	}

	return result;
}

static int fs_release(const char* path, struct fuse_file_info* fi)
{
	close(fi->fh);
	return 0;
}

static struct fuse_operations fs_impl =
{
	.getattr	= fs_getattr,
	.readdir	= fs_readdir,
	.open		= fs_open,
	.read		= fs_read,
	.release	= fs_release
};

filesystem_h filesystem_create(void)
{
	filesystem_h handle = (filesystem_h) calloc(1, sizeof(struct filesystem_s));
	ASSERT_RET(handle != NULL, NULL);

	handle->devices = g_hash_table_new_full(g_str_hash, g_str_equal, free, (GDestroyNotify) db_unref);

	return handle;
}

void filesystem_run(filesystem_h handle, const char* location)
{
	ASSERT_RET(handle != NULL);
	ASSERT_RET(location != NULL);

	fs_instance = handle;

	const char* params[] = { "ipa", "-f", location };
	fuse_main(sizeof(params) / sizeof(params[0]), (char **) params, &fs_impl, NULL);
}

bool filesystem_add_database(filesystem_h handle, db_h database)
{
	ASSERT_RET(handle != NULL, false);
	ASSERT_RET(database != NULL, false);
	ASSERT_RET(db_get_device_name(database) != NULL, false);

	char* device_name = strdup(db_get_device_name(database));

	// guarantee unique name of the device
	if (g_hash_table_contains(handle->devices, device_name))
	{
		// attempt subsequent names until a free one is found
		uint32_t suffix = 1;

		while (true)
		{
			if (device_name != NULL)
			{
				free(device_name);
				device_name = NULL;
			}

			asprintf(&device_name, "%s (%d)", ++suffix);
			if (!g_hash_table_contains(handle->devices, device_name))
			{
				// a unique name has been found
				break;
			}
		}
	}

	g_hash_table_insert(handle->devices, device_name, database);
	return true;
}

db_h filesystem_get_database_by_fs_name(filesystem_h handle, const char* fs_name)
{
	ASSERT_RET(handle != NULL, NULL);
	ASSERT_RET(fs_name != NULL, NULL);

	db_h db = (db_h) g_hash_table_lookup(handle->devices, fs_name);

	if (db == NULL)
	{
		return NULL;
	}

	return db_ref(db);
}

void filesystem_free(filesystem_h handle)
{
	if (handle)
	{
		g_hash_table_unref(handle->devices);
		free(handle);
		fs_instance = NULL;
	}
}
