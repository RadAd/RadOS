
#pragma pack (push, 0);
struct boot_sector
{
    BYTE jump[3];
    char oem[8];
    WORD bytes_per_sector;
    BYTE sector_per_cluster;
    WORD reserved_sectors;
    BYTE number_of_fats;
    WORD number_of_root_directory_entries;
    WORD number_of_sectors;
    BYTE media_descriptor;
    WORD sectors_per_fat;
    WORD sectors_per_track;
    WORD number_of_heads;
    WORD number_of_hidden_sectors;
};
#pragma pack (pop);

#pragma pack (push, 0);
struct directory_entry
{
    char name[8];
    char ext[3];
    BYTE attribute;
    BYTE reserved[10];
    WORD time;
    WORD date;
    WORD start_cluster;
    DWORD file_size; // bytes
};
#pragma pack (pop);

enum FileAttribute
{
    FILE_ATTR_READ_ONLY,
    FILE_ATTR_HIDDEN,
    FILE_ATTR_SYSTEM,
    FILE_ATTR_VOLUME,
    FILE_ATTR_DIRECTORY,
    FILE_ATTR_ARCHIVE,
};
