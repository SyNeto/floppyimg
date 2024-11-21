#include "fat12lib.h"
#include <stdio.h>
#include <stdlib.h>


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

    if (load_fat_entries(image_file, &boot_sector, &fat_entries) != 0)
    {
        fclose(image_file);
        return -1;
    }

    printf("FAT_SIZE: %d bytes\n", FAT_SIZE(&boot_sector));
    printf("Root entries count: %d\n", boot_sector.root_entries_count);
    printf("Root dir size: %d\n", ROOT_DIR_SIZE(&boot_sector));
    printf("Data area start: %d\n", DATA_AREA_START(&boot_sector));
    printf("Data cluster offset: %d\n", DATA_CLUSTER_OFFSET(&boot_sector));
    fclose(image_file);
    free(fat_entries.entries);

    return 0;
}
