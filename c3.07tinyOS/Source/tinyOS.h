#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>
#include "tLib.h"
#include "tConfig.h"

#define NVIC_INIT_CTRL			0xE000ED04	//The address of NVIC_INIT_CTRL register
#define NVIC_INIT_PENDSVSET		0x10000000	//The value to be written to the NVIC_INIT_CTRL

#define NVIC_INIT_SYSPRI2		0xE000ED22	//The address of NVIC_INIT_SYSPRI2 register
#define NVIC_INIT_PENDSV_PRI	0x000000FF	//The value to be written to the NVIC_INIT_PENDSV_PRI

#define MEM8(addr)		*(volatile unsigned char *)addr
#define MEM32(addr)		*(volatile unsigned long *)addr
	
#define TINYOS_TASK_STATE_RDY    0
#define TINYOS_TASK_STATE_DELAY  (1 << 1)


typedef uint32_t tTaskStack;

typedef struct _tTask{
	tTaskStack* stack;
	uint32_t delayTicks; 
	uint32_t prio;
	tNode linkNode;
	tNode delayNode;
	uint32_t state;
	uint32_t slice;	
}tTask;


extern tTask* currentTask;
extern tTask* nextTask;
extern tList taskTable[TINYOS_PRIO_COUNT];

void tTaskInit(tTask* task, void(*entry)(void*), void* param, uint32_t prio, tTaskStack* stack);
void tTaskSwitch(void);
void tTaskRunFirst(void);
void  tTaskExitCritical(uint32_t status);
uint32_t tTaskEnterCritical(void);
void tTaskSchedInit(void);
void tTaskSchedDisable(void);
void tTaskSchedEnable(void);


#endif
