#include "shell.h"

#include "fatfs.h"
#include "bool.h"
#include "terminal.h"

#include <stdlib.h>
#include <string.h>

inline BOOL isempty(const char* s)
{
	return s[0] == '\0';
}

inline const TCHAR* get_filename(const TCHAR* file)
{
	const TCHAR* f1 = strrchr(file, '\\');
	const TCHAR* f2 = strrchr(file, '/');
	if (f1 == NULL) f1 = file; else ++f1;
	if (f2 == NULL) f2 = file; else ++f2;
    return f1 > f2 ? f1 : f2;
}

FRESULT printdir(const TCHAR* dir)
{
	DIR d = { 0 };
	FILINFO fi = { 0 };
	FRESULT r;
	
	if (dir == NULL)
        dir = ".";

	r = f_opendir(&d, dir);
	if (r != FR_OK)
		return r;
	while (((r = f_readdir(&d, &fi)) == FR_OK) && (!isempty(fi.fname)))
	{
		const struct FatTime ftime = UnpackTime(fi.ftime);
		const struct FatDate fdate = UnpackDate(fi.fdate);

		terminal_print_fmt("%c%c%c%c%c %-11s %8lu %02d/%02d/%04d %02d:%02d:%02d\n",
			fi.fattrib & AM_DIR ? 'd' : '-', fi.fattrib & AM_RDO ? 'r' : '-', fi.fattrib & AM_ARC ? 'a' : '-', fi.fattrib & AM_SYS ? 's' : '-', fi.fattrib & AM_HID ? 'h' : '-',
			fi.fname, fi.fsize, fdate.day, fdate.month, fdate.year + 1980, ftime.hours, ftime.minutes, ftime.seconds * 2);
	}
	if (r != FR_OK)
	{
		f_closedir(&d);
		return r;
	}
	r = f_closedir(&d);
	return r;
}

FRESULT printfile(const TCHAR* file)
{
	FIL f = { 0 };
	FRESULT r = f_open(&f, file, FA_READ | FA_OPEN_EXISTING);
	if (r != FR_OK)
		return r;

	while (TRUE)
	{
		BYTE buf[1025];
		UINT br = 0;
		r = f_read(&f, buf, sizeof(buf) - 1, &br);
		if (r != FR_OK || br == 0)
			break;

		buf[br] = '\0';
		terminal_print_str((const char*) buf);
	}

	f_close(&f);

	terminal_print_str("\n");

	return r;
}

FRESULT copyfile(const TCHAR* src, const TCHAR* dst)
{
    FIL fsrc = { 0 };
    FIL fdst = { 0 };
    FRESULT r;
    
    if (dst == NULL)
        dst = get_filename(src);
    
    r = f_open(&fsrc, src, FA_READ | FA_OPEN_EXISTING);
    if (r != FR_OK)
        return FR_NO_FILE;

    r = f_open(&fdst, dst, FA_WRITE | FA_CREATE_ALWAYS | FA_CREATE_NEW);
    if (r != FR_OK)
    {
        f_close(&fsrc);
        return r;
    }

	while (TRUE)
	{
		BYTE buf[1024];
		UINT br = 0;
		UINT bw = 0;
		
		r = f_read(&fsrc, buf, sizeof(buf), &br);
		if (r != FR_OK || br == 0)
			break;

		r = f_write(&fdst, buf, br, &bw);
		if (r != FR_OK)
			break;
	}

    f_close(&fdst);

    f_close(&fsrc);

    return r;
}

void shell_chdir(const char* dir)
{
    FRESULT r;
    r = f_chdir(dir);
    if (r != FR_OK)
        terminal_print_fmt("ERROR: f_chdir: %d\n", r);
}

void shell_type(const char* file)
{
    FRESULT r;
    r = printfile(file);
    if (r != FR_OK)
        terminal_print_fmt("ERROR: printfile: %d\n", r);
}

void shell_copy(const char* src, const char* dst)
{
    FRESULT r;
    r = copyfile(src, dst);
    if (r != FR_OK)
        terminal_print_fmt("ERROR: printdir: %d\n", r);
}

void shell_del(const char* file)
{
    FRESULT r;
    r = f_unlink(file);
    if (r != FR_OK)
        terminal_print_fmt("ERROR: f_unlink: %d\n", r);
}

void shell_dir(const char* dir)
{
    FRESULT r;
    r = printdir(dir);
    if (r != FR_OK)
        terminal_print_fmt("ERROR: printdir: %d\n", r);
}

void shell_mkdir(const char* dir)
{
    FRESULT r;
    r = f_mkdir(dir);
    if (r != FR_OK)
        terminal_print_fmt("ERROR: f_mkdir: %d\n", r);
}

void shell_rmdir(const char* dir)
{
    FRESULT r;
    r = f_rmdir(dir);
    if (r != FR_OK)
        terminal_print_fmt("ERROR: f_rmdir: %d\n", r);
}
