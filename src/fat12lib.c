#include <stdlib.h>
#include "fat12lib.h"

int parse_boot_sector(FILE *file, BootSector *boot_sector)
{
    if (!file)
    {
        fprintf(stderr, "Error: Invalid file pointer\n");
        return -1;
    }
    if (fread(boot_sector, sizeof(BootSector), 1, file) != 1)
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
    if (fread(compact_fat, 1, fat_size, file) != fat_size)
    {
        fprintf(stderr, "Error: Unable to read FAT entries\n");
        free(fat_entries->entries);
        free(compact_fat);
        fat_entries->entries = NULL;
        fat_entries->size = 0;
    }

    for (uint16_t i = 0; i < cluster_count; i++)
    {
        if (i % 2 == 0)
        {
            fat_entries->entries[i] = compact_fat[i * 3 / 2] | ((compact_fat[i * 3 / 2 + 1] & 0x0F) << 8);
        }
        else
        {
            fat_entries->entries[i] = ((compact_fat[i * 3 / 2] >> 4) & 0x0F) | (compact_fat[i * 3 / 2 + 1] << 4);
        }
    }

    free(compact_fat);
    return 0;
}

/**
 * @brief validate the fat entry
 *
 * @param entry an uncompressed FAT entry (12bits -> 16bits)
 * @param fat_size the size of the FAT table
 */
static int _is_valid_fat_entry(uint16_t entry, uint16_t fat_size)
{
    if (entry == 0x000)
    {
        return 1;
    }
    else if (entry >= 0xFF8 && entry <= 0xFFF)
    {
        return 1;
    }
    else if (entry == 0xFF7)
    {
        return 0;
    }
    else if (entry >= 2 && entry < fat_size)
    {
        return 1;
    }
    return 0;
}

static int _validate_fat_chain(const FatEntries *fat_entries, uint16_t start_cluster)
{
    uint16_t current_cluster = start_cluster;

    if (!fat_entries || !fat_entries->entries)
    {
        fprintf(stderr, "Error: Invalid FAT entries\n");
        return -1;
    }

    if (start_cluster >= fat_entries->size || start_cluster < 2)
    {
        fprintf(stderr, "Error: Invalid start cluster (must be >= 2 and < %d)\n", fat_entries->size);
        return -1;
    }

    uint8_t *visited = calloc(fat_entries->size, sizeof(uint8_t));
    if (!visited)
    {
        fprintf(stderr, "Error: Unable to allocate memory for visited clusters\n");
        return -1;
    }

    while (1)
    {
        if (!_is_valid_fat_entry(fat_entries->entries[current_cluster], fat_entries->size))
        {
            fprintf(stderr, "Error: Invalid FAT entry at cluster %u\n", current_cluster);
            free(visited);
            return -1;
        }

        if (visited[current_cluster])
        {
            fprintf(stderr, "Error: FAT chain contains a loop at cluster %u\n", current_cluster);
            free(visited);
            return -1;
        }
        visited[current_cluster] = 1;

        uint16_t next_cluster = fat_entries->entries[current_cluster];
        if (next_cluster >= 0xFF8 && next_cluster <= 0xFFF)
        {
            free(visited);
            break;
        }
        current_cluster = next_cluster;
    }
    return 0;
}

int parse_fat_table(const FatEntries *fat_entries, FatChains *fat_chains)
{
    if (!fat_entries || !fat_entries->entries)
    {
        fprintf(stderr, "Error: Invalid FAT entries\n");
        return -1;
    }

    uint32_t *visited = calloc(fat_entries->size, sizeof(uint32_t));
    if (!visited)
    {
        fprintf(stderr, "Error: Unable to allocate memory for visited clusters\n");
        return -1;
    }

    size_t max_chains = 10;
    fat_chains->chains = malloc(max_chains * sizeof(uint16_t *));
    fat_chains->lengths = malloc(max_chains * sizeof(size_t));
    fat_chains->count = 0; // Asegúrate de inicializar `count` a 0.
    if (!fat_chains->chains || !fat_chains->lengths)
    {
        fprintf(stderr, "Error: Unable to allocate memory for FAT chains\n");
        free(visited);
        free(fat_chains->chains);
        free(fat_chains->lengths);
        return -1;
    }

    for (uint16_t i = 2; i < fat_entries->size; i++)
    {
        if (visited[i] || fat_entries->entries[i] == 0x000)
        {
            continue; // Skip unused clusters.
        }

        if(_validate_fat_chain(fat_entries, i) != 0)
        {
            // Invalid FAT chain, skip to next cluster
            continue;
        }

        uint16_t current_cluster = i;
        uint16_t *current_chain = NULL;
        size_t chain_length = 0;

        while (current_cluster < fat_entries->size && !visited[current_cluster])
        {
            visited[current_cluster] = 1;
            chain_length++;
            current_chain = realloc(current_chain, chain_length * sizeof(uint16_t));
            if (!current_chain)
            {
                fprintf(stderr, "Error: Unable to allocate memory for FAT chain\n");
                free(visited);
                return -1;
            }
            current_chain[chain_length - 1] = current_cluster;

            uint16_t next_cluster = fat_entries->entries[current_cluster];
            if (next_cluster >= 0xFFF) // End of chain.
            {
                break;
            }
            current_cluster = next_cluster;
        }

        if (fat_chains->count >= max_chains)
        {
            max_chains *= 2;
            fat_chains->chains = realloc(fat_chains->chains, max_chains * sizeof(uint16_t *));
            fat_chains->lengths = realloc(fat_chains->lengths, max_chains * sizeof(size_t));
            if (!fat_chains->chains || !fat_chains->lengths)
            {
                fprintf(stderr, "Error: Unable to resize FAT chains\n");
                free(visited);
                return -1;
            }
        }

        // Agregar la nueva cadena al struct.
        fat_chains->chains[fat_chains->count] = current_chain;
        fat_chains->lengths[fat_chains->count] = chain_length;
        fat_chains->count++;
    }

    free(visited);
    return 0;
}
