#include <stdlib.h>
#include "fat12lib.h"

int parse_boot_sector(FILE *file, BootSector *boot_sector)
{
    if (!file)
    {
        fprintf(stderr, "Error: Invalid file pointer\n");
        return -1;
    }
    if( fread(boot_sector, sizeof(BootSector), 1, file) != 1)
    {
        fprintf(stderr, "Error: Unable to read boot sector\n");
        return -1;
    }
    return 0;
}

int load_fat_entries(FILE *file, BootSector *boot_sector, FatEntries *fat_entries)
{
    uint16_t fat_sart = FAT_START(boot_sector);
    uint16_t fat_size = FAT_SIZE(boot_sector);
    uint16_t cluster_count = DATA_CLUSTERS(boot_sector);

    fat_entries->entries = malloc(cluster_count * sizeof(uint16_t));
    if (!fat_entries->entries)
    {
        fprintf(stderr, "Error: Unable to allocate memory for FAT entries\n");
        return -1;
    }
    fat_entries->size = cluster_count;

    uint8_t *compact_fat = malloc(fat_size);
    if (!compact_fat)
    {
        fprintf(stderr, "Error: Unable to allocate memory for compacted FAT data\n");
        free(fat_entries->entries);
        fat_entries->entries = NULL;
        fat_entries->size = 0;
        return -1;
    }

    fseek(file, fat_sart, SEEK_SET);
    if(fread(compact_fat, 1, fat_size, file) != fat_size){
        fprintf(stderr, "Error: Unable to read FAT entries\n");
        free(fat_entries->entries);
        free(compact_fat);
        fat_entries->entries = NULL;
        fat_entries->size = 0;
    }

    for(uint16_t i = 0; i < cluster_count; i++)
    {
        if(i % 2 == 0)
        {
            fat_entries->entries[i] = compact_fat[i * 3 / 2] | ((compact_fat[i * 3 / 2 + 1] & 0x0F) << 8);
        } else {
            fat_entries->entries[i] = ((compact_fat[i * 3 / 2] >> 4) & 0x0F) | (compact_fat[i * 3 / 2 + 1] << 4);
        }
    }

    free(compact_fat);
    return 0;
}

int traverse_fat_chain(const FatEntries *fat_entries, uint16_t start_cluster)
{
    if (start_cluster >= fat_entries->size || start_cluster == 0)
    {
        fprintf(stderr, "Error: Invalid start cluster\n");
        return -1;
    }
    return 0;
}
