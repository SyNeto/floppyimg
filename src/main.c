#include "fat12lib.h"
#include <stdio.h>

int main(void){
    BootSector boot_sector;
    FILE *image_file = fopen("disk.img", "rb");
    if (!image_file) {
        fprintf(stderr, "Error: Unable to open disk image\n");
        return -1;
    }

    if(parse_boot_sector(image_file, &boot_sector) == 0) {
        printf("OEM Name: %s\n", boot_sector.oem_name);
        printf("Bytes per sector: %d\n", boot_sector.bytes_per_sector);
        printf("Sectors per cluster: %d\n", boot_sector.sectors_per_cluster);
        printf("Reserved sectors: %d\n", boot_sector.reserved_sectors);
        printf("FAT count: %d\n", boot_sector.fat_count);
        printf("Root entries count: %d\n", boot_sector.root_entries_count);
        printf("Total sectors 16: %d\n", boot_sector.total_sectors_16);
        printf("Media type: %d\n", boot_sector.media_type);
        printf("Sectors per FAT: %d\n", boot_sector.sectors_per_fat);
        printf("Sectors per track: %d\n", boot_sector.sectors_per_track);
        printf("Number of heads: %d\n", boot_sector.num_heads);
        printf("Hidden sectors: %d\n", boot_sector.hidden_sectors);
        printf("Total sectors 32: %d\n", boot_sector.total_sectors_32);
    }
    fclose(image_file);
    printf("Total disk size: %d bytes \t (size of type: %zu bytes)\n", TOTAL_DISK_SIZE(boot_sector), sizeof(TOTAL_DISK_SIZE(boot_sector)));
    printf("Reserved size: %d bytes\n", RESERVED_SIZE(boot_sector));
    printf("FAT size: %d bytes\n", FAT_SIZE(boot_sector));
    printf("Root directory size: %d bytes\n", ROOT_DIR_SIZE(boot_sector));
    printf("System area size: %d bytes\n", SYSTEM_AREA_SIZE(boot_sector));
    printf("FAT start: %d bytes\n", FAT_START(boot_sector));
    printf("Cluster count: %d\n", TOTAL_CLUSTERS(boot_sector));
    printf("System cluster count: %d\n", SYSTEM_CLUSTERS(boot_sector));
    printf("Data cluster count: %d\n", DATA_CLUSTERS(boot_sector));
    return 0;
}
