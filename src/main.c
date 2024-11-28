#include "fat12lib.h"
#include <stdio.h>
#include <stdlib.h>

void print_binary(uint16_t value, size_t size)
{
    for (size_t i = size; i > 0; i--)
    {
        printf("%c", (value & (1 << (i - 1))) ? '1' : '0');
    }
}

void print_fat_chains(FatChains *fat_chains)
{
    if (!fat_chains || !fat_chains->chains || !fat_chains->lengths)
    {
        fprintf(stderr, "Error: Invalid FAT chains\n");
        return;
    }

    printf("Reading FAT chains...\n");
    printf("Chain | Length | Clusters\n");
    printf("-------------------------\n");
    for (size_t i = 0; i < fat_chains->count; i++)
    {
        printf("%5zu | %6zu | ", i, fat_chains->lengths[i]);
        for (size_t j = 0; j < fat_chains->lengths[i]; j++)
        {
            printf("%u ", fat_chains->chains[i][j]);
        }
        printf("\n");
    }
}

void explore_fat_table(const FatEntries *fat_entries)
{
    if (!fat_entries || !fat_entries->entries)
    {
        fprintf(stderr, "Error: Invalid FAT entries\n");
        return;
    }

    printf("Reading FAT entries...\n");
    printf("Cluster | Value (dec) | Value (hex) | Value (bin) | Status\n");
    printf("----------------------------------------------------------\n");
    for (size_t i = 0; i < fat_entries->size; i++)
    {
        uint16_t value = fat_entries->entries[i];
        printf("%6zu | %10u | 0x%04X    | ", i, value, value);
        print_binary(value, 12);
        printf(" | ");
        if (value == 0x000)
        {
            printf("Cluster %zu: Unused\n", i);
        } else if (value >= 0xFF8) {
            printf("Cluster %zu: Last cluster, end of chain\n", i);
        } else if (value == 0xFF7) {
            printf("Cluster %zu: Bad cluster\n", i);
        } else if (value >= 0xFF0) {
            printf("Cluster %zu: Reserved cluster\n", i);
        } else {
            printf("Cluster %zu: Next cluster is %u\n", i, value);
        }
    }
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s <disk image>\n", argv[0]);
        return -1;
    }

    BootSector boot_sector;
    FatEntries fat_entries;
    FatChains fat_chains = {0};

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
    printf("-------------------------------------\n");
    printf("Macros Test\n");
    printf("ROOT_DIR_SIZE: %d\n", ROOT_DIR_SIZE(&boot_sector));
    printf("DATA_AREA_START: %d\n", DATA_AREA_START(&boot_sector));
    printf("DATA_AREA_START_BYTES: %d\n", DATA_AREA_START_BYTES(&boot_sector));
    printf("DATA_CLUSTER_OFFSET (5): %d\n", DATA_CLUSTER_OFFSET(&boot_sector, 5));
    printf("CLUSTER_OFFSET (5): %d\n", CLUSTER_OFFSET(&boot_sector, 5));
    fclose(image_file);
    explore_fat_table(&fat_entries);
    parse_fat_table(&fat_entries, &fat_chains);
    printf("-------------------------------------\n");
    printf("FAT Chains\n");
    printf("Number of chains: %zu\n", fat_chains.count);
    print_fat_chains(&fat_chains);
    free(fat_chains.chains);
    free(fat_chains.lengths);
    free(fat_entries.entries);

    return 0;
}
