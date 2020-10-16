#include "fatfs.h"

struct FileInfo
{
	BYTE dev;
	FATFS fs;
};

struct FileInfo files[FF_VOLUMES] = { 255 };

FRESULT mount(BYTE pdrv, BYTE dev)
{
	struct FileInfo* file = &files[pdrv];
	char path[] = "0:";
	file->dev = dev;
	path[0] += pdrv;
	//terminal_print_fmt("mount %d %d\n", pdrv, file->dev);
	return f_mount(&file->fs, path, 1);
}

FRESULT unmount(BYTE pdrv)
{
	struct FileInfo* file = &files[pdrv];
	char path[] = "0:";
	file->dev = 255;
	path[0] += pdrv;
	return f_unmount(path);
}

BYTE get_dev(BYTE pdrv)
{
    return files[pdrv].dev;
}
