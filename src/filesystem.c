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

#define DEFAULT_MODE_ROOT S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH

static void getaatr_root(void* user_data)
{
	struct stat* stbuf = (struct stat*) user_data;
	stbuf->st_mode = DEFAULT_MODE_ROOT;
	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
}

static void getaatr_device(const db_h db, void* user_data)
{
	struct stat* stbuf = (struct stat*) user_data;
	lstat(db_get_root_path(db), stbuf);
}

static int fs_getattr(const char* path, struct stat* stbuf)
{
	ASSERT_RET(fs_instance != NULL, -ENOENT);
	ASSERT_RET(path != NULL, -ENOENT);

	LOG_DEBUG("(fs_getattr at %s)", path);

	if (path_parser_execute(path, fs_instance, (path_parser_cb_t) {
		.on_root = getaatr_root,
		.on_device = getaatr_device,
		.on_root_user_data = stbuf,
		.on_device_user_data = stbuf
	}))
	{
		return 0;
	}

	// todo: add other types of dirs

	// TODO: under other types of dirs you might want to call lstat (e.g. on the root of different devices) - more accurate data without hustle

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

static int fs_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info* fi)
{
	ASSERT_RET(fs_instance != NULL, -ENOENT);
	ASSERT_RET(path != NULL, -ENOENT);

	LOG_DEBUG("(fs_readdir at %s)", path);

	fuse_readdir_params_t params = {
		.buf = buf,
		.filler = filler
	};

	if (path_parser_execute(path, fs_instance, (path_parser_cb_t) {
		.on_root = readdir_root,
		.on_root_user_data = &params
	}))
	{
		return 0;
	}

	// todo: add other types of dirs

	return -ENOENT;
}

static int fs_open(const char* path, struct fuse_file_info* fi)
{
	LOG_DEBUG("Not implemented");

	//todo: if you use file handles, you should also allocate any necessary structures and set fi->fh.
	//https://github.com/libfuse/libfuse/blob/master/example/fusexmp_fh.c
	return -ENOENT;
}

static int fs_read(const char* path, char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi)
{
	LOG_DEBUG("Not implemented");
	return -ENOENT;
}

static int fs_release(const char* path, struct fuse_file_info* fi)
{
	LOG_DEBUG("Not implemented");
	return -ENOENT;
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
