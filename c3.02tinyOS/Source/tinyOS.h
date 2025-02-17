#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>


#define NVIC_INIT_CTRL			0xE000ED04	//The address of NVIC_INIT_CTRL register
#define NVIC_INIT_PENDSVSET		0x10000000	//The value to be written to the NVIC_INIT_CTRL

#define NVIC_INIT_SYSPRI2		0xE000ED22	//The address of NVIC_INIT_SYSPRI2 register
#define NVIC_INIT_PENDSV_PRI	0x000000FF	//The value to be written to the NVIC_INIT_PENDSV_PRI

#define MEM8(addr)		*(volatile unsigned char *)addr
#define MEM32(addr)		*(volatile unsigned long *)addr


typedef uint32_t tTaskStack;

typedef struct _tTask{
	tTaskStack* stack;
	uint32_t delayTicks; 
}tTask;


extern tTask* currentTask;
extern tTask* nextTask;
extern tTask* taskTable[2];

void tTaskSwitch(void);
void tTaskRunFirst(void);
void  tTaskExitCritical(uint32_t status);
uint32_t tTaskEnterCritical(void);
void tTaskSchedInit(void);
void tTaskSchedDisable(void);
void tTaskSchedEnable(void);


#endif
