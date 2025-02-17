#include "tinyOS.h"
#include "ARMCM3.h"

tTask *currentTask;											
tTask *nextTask;											
tTask *idleTask;											
tList taskTable[TINYOS_PRO_COUNT];							
tTask tTaskIdle;											
tTaskStack idleTaskEnv[TINYOS_IDLETASK_STACK_SIZE];			

uint8_t schedLockCount;								
tBitmap taskPrioBitmap;								
tList tTaskDelayedList;								

void idleTaskEntry(void *param);																	

int main(void)
 {
	tTaskSchedInit();
	tTaskDelayInit();																				
	tTimerModuleInit();
	tAppInit(); 
	
	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, TINYOS_PRO_COUNT-1, 
 						idleTaskEnv,TINYOS_IDLETASK_STACK_SIZE);						
	
	idleTask=&tTaskIdle;
	
	nextTask=tTaskHighestReady();			
	tTaskRunFirst();
	return 0;
}

void idleTaskEntry(void *param)				
{
	for(;;)
	{
	}
}
