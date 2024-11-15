#include "fat12lib.h"

int parse_boot_sector(FILE *file, BootSector *boot_sector)
{
    if (!file)
    {
        fprintf(stderr, "Error: Invalid file pointer\n");
        return -1;
    }
    fread(boot_sector, sizeof(BootSector), 1, file);
    return 0;
}
