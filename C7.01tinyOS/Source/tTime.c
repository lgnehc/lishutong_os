#include "tinyOS.h"

void tTaskDelayInit(void)									//Task delayed list initial function
{
	tListInit(&tTaskDelayedList);
}

void tTaskDelay(uint32_t delay)								//Task delay function, the parameter ms must be integral multiple of 10 
{
	uint32_t status=tTaskEnterCritical();
	
	tTimeTaskWait(currentTask, delay);
	
	tTaskSchedUnRdy(currentTask);
	
	tTaskExitCritical(status);
	
	tTaskSched();
}
