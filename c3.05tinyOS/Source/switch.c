#include "tinyos.h"
#include "ARMCM3.h"


__asm void PendSV_Handler(void)
{
	IMPORT currentTask;
	IMPORT nextTask;
	
	MRS R0,PSP
	CBZ R0,PendSVHandler_noSave
	
	STMDB R0!, {R4-R11}	//Push stack
	
	LDR R1, =currentTask
	LDR R1, [R1]
	STR R0, [R1]	//Update task structure stack pointer
	
PendSVHandler_noSave
	LDR R0, =currentTask;
	LDR R1, =nextTask;
	LDR R2,[R1]
	STR R2,[R0]
	
	LDR R0,[R2]
	LDMIA R0!,{R4-R11}
	
	MSR PSP, R0
	ORR LR,LR,#0x04
	
	BX LR
	
}

uint32_t tTaskEnterCritical(void)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	return primask;
}

void  tTaskExitCritical(uint32_t status)
{
	__set_PRIMASK(status);
}

void tTaskRunFirst()	
{
	__set_PSP(0); 
	MEM8(NVIC_INIT_SYSPRI2)=NVIC_INIT_PENDSV_PRI;		//Set the priority of PendSV
	MEM32(NVIC_INIT_CTRL)=NVIC_INIT_PENDSVSET;			//Directly trigger PendSV
}

void tTaskSwitch()	
{
	MEM32(NVIC_INIT_CTRL)=NVIC_INIT_PENDSVSET;			//Directly trigger PendSV
}
