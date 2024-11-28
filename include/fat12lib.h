#ifndef FAT12LIB_H
#define FAT12LIB_H

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Calculate the start of the FAT area in bytes
 */
#define FAT_START(boot) ((boot)->reserved_sectors * (boot)->bytes_per_sector)

/**
 * @brief Calculate the size of the FAT entries in bytes
 */
#define FAT_SIZE(boot) ((boot)->sectors_per_fat * (boot)->bytes_per_sector)

/**
 * @brief Calculate the size of the root directory in sectors
 */
#define ROOT_DIR_SIZE(boot) (((boot)->root_entries_count * 32) + (boot)->bytes_per_sector - 1) / (boot)->bytes_per_sector

/**
 * @brief Calculate the total number of clusters
 */
#define TOTAL_CLUSTERS(boot) ((boot)->total_sectors_16 / (uint16_t)(boot)->sectors_per_cluster)

/**
 * @brief Calculate the total number of clusters in the system area
 */
#define SYSTEM_CLUSTERS(boot) (                               \
    (boot)->reserved_sectors +                                \
    ((uint16_t)(boot)->fat_count * (boot)->sectors_per_fat) + \
    ((boot)->root_entries_count * 32) / (boot)->bytes_per_sector)

/**
 * @brief Calculate the total number of clusters in the data area
 */
#define DATA_CLUSTERS(boot) (TOTAL_CLUSTERS(boot) - SYSTEM_CLUSTERS(boot))

/**
 * @brief Calculate the start of the sectors of the data area
 */
#define DATA_AREA_START(boot) ((uint32_t)(boot)->reserved_sectors + ((uint32_t)(boot)->fat_count * (uint32_t)(boot)->sectors_per_fat) + (uint32_t)ROOT_DIR_SIZE(boot))

/**
 * @brief Calculate the start of the data area in bytes
 */
#define DATA_AREA_START_BYTES(boot) ((uint32_t)DATA_AREA_START(boot) * (uint32_t)(boot)->bytes_per_sector)


/**
 * @brief Calculate the offset of the data clusters in bytes
 */
#define DATA_CLUSTER_OFFSET(boot, cluster) (cluster - 2) * ((uint16_t)(boot)->sectors_per_cluster * (uint16_t)(boot)->bytes_per_sector)

/**
 * @brief Calculate the offset of the data clusters in bytes
 */
#define CLUSTER_OFFSET(boot, cluster) (DATA_AREA_START_BYTES(boot) + (uint32_t)DATA_CLUSTER_OFFSET(boot, cluster))

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
 * @brief FAT entries structure
 */
typedef struct
{
    uint16_t *entries; // FAT entries
    uint16_t size;     // Number of entries
} FatEntries;

/**
 * @brief FAT chains structure
 */
typedef struct
{
    uint16_t **chains;
    size_t *lengths;
    size_t count;
} FatChains;

/**
 * @brief Parse the boot sector of a FAT12 file system.
 *
 * @param file File pointer to the FAT12 image
 * @param boot_sector Pointer to the boot sector structure
 * @return 0 on success, -1 on error
 */
int parse_boot_sector(FILE *file, BootSector *boot_sector);

/**
 * @brief Parse the FAT entries of a FAT12 file system.
 *
 * @param file File pointer to the FAT12 image
 * @param boot_sector Pointer to the boot sector structure
 * @param fat_entries Pointer to the FAT entries structure
 * @return 0 on success, -1 on error
 */
int load_fat_entries(FILE *file, BootSector *boot_sector, FatEntries *fat_entries);

/**
 * @brief Analyze the FatEntries to find valid cluster chains.
 * 
 * @param fat_entries const Pointer to the FAT entries structure
 * @return 0 on success, -1 on error
 */
int parse_fat_table(const FatEntries *fat_entries, FatChains *fat_chains);

#endif
