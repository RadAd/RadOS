#include "fatfs/ff.h"

FRESULT mount(BYTE pdrv, BYTE dev);
FRESULT unmount(BYTE pdrv);

BYTE get_dev(BYTE pdrv);

#pragma pack(push)
#pragma pack(2)
struct FatTime
{
	unsigned short seconds : 5;
	unsigned short minutes : 6;
	unsigned short hours : 5;
};

struct FatDate
{
	unsigned short day : 5;
	unsigned short month : 4;
	unsigned short year : 7;
};

struct FatDateTime
{
	struct FatTime time;
	struct FatDate date;
};
#pragma pack(pop)

inline struct FatTime UnpackTime(WORD time)
{
	union
	{
		struct FatTime ftime;
		WORD wtime;
	} u;
	u.wtime = time;
	return u.ftime;
}

inline struct FatDate UnpackDate(WORD date)
{
	union
	{
		struct FatDate fdate;
		WORD wdate;
	} u;
	u.wdate = date;
	return u.fdate;
}
