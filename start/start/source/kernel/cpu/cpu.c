#include "cpu/cpu.h"
#include "os_cfg.h"
#include "comm/cpu_instr.h"


static segment_desc_t gdt_table[GDT_TABLE_SIZE];

void segment_desc_set(int selector,uint32_t base,uint32_t limit,uint16_t attr){
    //segment_desc_t *desc = gdt_table + (selector >> 3); 
    segment_desc_t *desc = gdt_table + selector/sizeof(segment_desc_t);

    if(limit > 0xFFFFF){
        attr |= 0x8000; 
        limit /= 0x1000;
    }

    desc->limit15_0 = limit & 0xFFFF;
    desc->base15_0  = base & 0xFFFF;
    desc->base23_16 = (base >> 16) & 0xFF;
    desc->base31_24 = (base >> 24) & 0xFF;
    desc->attr = attr | ( ((limit >> 16) & 0xF) << 8);
}

void init_gdt(void){
    for (int i = 0; i < GDT_TABLE_SIZE ; i++){
        segment_desc_set( i << 3, 0 , 0, 0 );
    }
    //hell
    
    segment_desc_set(KERNEL_SELECTOR_DS,0,0XFFFFFFFF,
        SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_DATA
        | SEG_TYPE_RW | SEG_D | SEG_G);
    segment_desc_set(KERNEL_SELECTOR_CS,0,0XFFFFFFFF,
        SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_CODE
        | SEG_TYPE_RW | SEG_D | SEG_G);


    lgdt((uint32_t)gdt_table,sizeof(gdt_table));
}

void cpu_init(void){
    init_gdt();
}
