#ifndef FAT12LIB_H
#define FAT12LIB_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Calculate the total disk size
 */
#define TOTAL_DISK_SIZE(boot) (boot.total_sectors_16 * boot.bytes_per_sector)

/**
 * @brief Calculate the size of the reserved area
 */
#define RESERVED_SIZE(boot) (boot.reserved_sectors * boot.bytes_per_sector)

/**
 * @brief Calculate the size of the FAT area
 */
#define FAT_SIZE(boot) (boot.sectors_per_fat * boot.fat_count * boot.bytes_per_sector)

/**
 * @brief Calculate the size of the root directory
 */
#define ROOT_DIR_SIZE(boot) ((boot.root_entries_count * 32) + (boot.bytes_per_sector - 1)) / boot.bytes_per_sector *boot.bytes_per_sector

/**
 * @brief Calculate the size of the system area
 */
#define SYSTEM_AREA_SIZE(boot) (RESERVED_SIZE(boot) + FAT_SIZE(boot) + ROOT_DIR_SIZE(boot))

/**
 * @brief Calculate the start of the FAT area
 */
#define FAT_START(boot) (boot.reserved_sectors * boot.bytes_per_sector)

/**
 * @brief Calculate the total number of clusters
 */
#define TOTAL_CLUSTERS(boot) (boot.total_sectors_16 / (uint16_t)boot.sectors_per_cluster)

/**
 * @brief Calculate the total number of clusters in the system area
 */
#define SYSTEM_CLUSTERS(boot) (boot.reserved_sectors + ((uint16_t)boot.fat_count * boot.sectors_per_fat) + (boot.root_entries_count * 32) / boot.bytes_per_sector)

/**
 * @brief Calculate the total number of clusters in the data area
 */
#define DATA_CLUSTERS(boot) (TOTAL_CLUSTERS(boot) - SYSTEM_CLUSTERS(boot))

/**
 * @brief FAT12 boot sector structure
 */
typedef struct
{
    uint8_t jump[3];                  // Jump instruction (3 bytes)
    char oem_name[8];                 // OEM Name (8 bytes)
    uint16_t bytes_per_sector;        // Bytes per sector (2 bytes)
    uint8_t sectors_per_cluster;      // Sectors per cluster (1 byte)
    uint16_t reserved_sectors;        // Reserved sectors (2 bytes)
    uint8_t fat_count;                // Number of FATs (1 byte)
    uint16_t root_entries_count;      // Number of root directory entries (2 bytes)
    uint16_t total_sectors_16;        // Total sectors (2 bytes)
    uint8_t media_type;               // Media type (1 byte)
    uint16_t sectors_per_fat;         // Sectors per FAT (2 bytes)
    uint16_t sectors_per_track;       // Sectors per track (2 bytes)
    uint16_t num_heads;               // Number of heads (2 bytes)
    uint32_t hidden_sectors;          // Hidden sectors (4 bytes)
    uint32_t total_sectors_32;        // Total sectors if > 65535 (4 bytes)
} __attribute__((packed)) BootSector; // GCC specific attribute to prevent padding

/**
 * @brief FAT entry type
 * 
 * The FAT entry is a 12-bit value that represents the next cluster in the chain.
 * 
 * This type is a 16-bit representation where the lower 12 bits are used to
 * store the next cluster.
 */
typedef uint16_t fat_t;

/**
 * @brief Parse the boot sector of a FAT12 file system.
 */
int parse_boot_sector(FILE *file, BootSector *boot_sector);

// fat_t *parse_fat(FILE *file, uint16_t fat_start, fat_size)

#endif
