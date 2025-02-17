/**
 * 自己动手写操作系统
 *
 * 系统引导部分，启动时由硬件加载运行，然后完成对二级引导程序loader的加载
 * boot扇区容量较小，仅512字节。由于dbr占用了不少字节，导致其没有多少空间放代码，
 * 所以功能只能最简化,并且要开启最大的优化-os
 *
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef LOADER_H
#define LOADER_H

#include "comm/types.h"
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"

void protect_mode_entry(void);

// 内存检测信息结构
typedef struct SMAP_entry {
    uint32_t BaseL; // base address uint64_t
    uint32_t BaseH;
    uint32_t LengthL; // length uint64_t
    uint32_t LengthH;
    uint32_t Type; // entry Type
    uint32_t ACPI; // extended
}__attribute__((packed)) SMAP_entry_t;


extern  boot_info_t boot_info;

#endif
