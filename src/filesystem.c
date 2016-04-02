#include "filesystem.h"
#include "logger.h"

#define FUSE_USE_VERSION 29

#include <fuse.h>
#include <errno.h>
#include <stdio.h>

static int fs_getattr(const char* path, struct stat* stbuf)
{
	LOG_DEBUG("Not implemented");
	return -ENOENT;
}

static int fs_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info* fi)
{
	LOG_DEBUG("Not implemented");
	return -ENOENT;
}

static int fs_open(const char* path, struct fuse_file_info* fi)
{
	LOG_DEBUG("Not implemented");
	return -ENOENT;
}

static int fs_read(const char* path, char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi)
{
	LOG_DEBUG("Not implemented");
	return -ENOENT;
}

static struct fuse_operations fs_impl =
{
	.getattr	= fs_getattr,
	.readdir	= fs_readdir,
	.open		= fs_open,
	.read		= fs_read
};

void filesystem_run(char* location)
{
	if (location == NULL)
	{
		LOG_WARN("The location parameter is incorrect");
		return;
	}

	char* params[] = { "ipa", "-f", location };
	fuse_main(sizeof(params) / sizeof(params[0]), params, &fs_impl, NULL);
}
