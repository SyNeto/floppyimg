#include "fat12lib.h"
#include <stdio.h>

void print_binary(uint16_t num) {
    for (int i = 15; i >= 0; i--) {
        printf("%d", (num >> i) & 1);
        if(i % 8 == 0) {
            printf("\t");
        }
    }
}

void print_hex(uint16_t num) {
    printf("\t%x", num);
}

void print_fat_entries(FatEntries *fat_entries)
{
    for (uint16_t i = 0; i < fat_entries->size; i++)
    {
        printf("FAT entry %d: \t", i);
        print_binary(fat_entries->entries[i]);
        print_hex(fat_entries->entries[i]);
        printf("\n");
    }
}

int main(void){
    BootSector boot_sector;
    FatEntries fat_entries;
    FILE *image_file = fopen("disk.img", "rb");
    if (!image_file) {
        fprintf(stderr, "Error: Unable to open disk image\n");
        return -1;
    }

    if (parse_boot_sector(image_file, &boot_sector) != 0) {
        fclose(image_file);
        return -1;
    }

    if (parse_fat_entries(image_file, &boot_sector, &fat_entries) != 0) {
        fclose(image_file);
        return -1;
    }

    print_fat_entries(&fat_entries);

    printf("Bytes per sector: %d\n", boot_sector.bytes_per_sector);
    fclose(image_file);

    return 0;
}
