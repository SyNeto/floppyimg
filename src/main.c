#include "fat12lib.h"
#include <stdio.h>


int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s <disk image>\n", argv[0]);
        return -1;
    }
    BootSector boot_sector;
    FatEntries fat_entries;
    FILE *image_file = fopen(argv[1], "rb");
    if (!image_file)
    {
        fprintf(stderr, "Error: Unable to open disk image\n");
        return -1;
    }

    if (parse_boot_sector(image_file, &boot_sector) != 0)
    {
        fclose(image_file);
        return -1;
    }

    if (parse_fat_entries(image_file, &boot_sector, &fat_entries) != 0)
    {
        fclose(image_file);
        return -1;
    }

    printf("Bytes per sector: %d\n", boot_sector.bytes_per_sector);
    fclose(image_file);

    return 0;
}
