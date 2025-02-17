#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include "types.h"

#define BOOT_RAM_REGINE_MAX 10

typedef struct _boot_info_t{
    struct {
        uint32_t start;
        uint32_t size;

    }ram_region_cfg[BOOT_RAM_REGINE_MAX];

    int ram_region_count;

}boot_info_t;

#define SECTOR_SIZE 512
#define SYS_KERNEL_LOAD_ADDR (1024*1024)

#endif