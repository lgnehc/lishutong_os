#include "tinyOS.h"
#include "ARMCM3.h"

int task1Flag;	//Task1 flag
int task2Flag;	//Task2 flag

tTask tTask1;
tTask tTask2;
tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];

tTask* currentTask;
tTask* nextTask;
tTask* idleTask;
tTask* taskTable[2];



void tTaskInit(tTask* task, void(*entry)(void*), void* param, tTaskStack* stack);
void tSetSysTickPeriod(uint32_t ms);
void tTaskSched();
void task1Entry(void *param);
void task2Entry(void *param);
void tTaskSystemTickHandler(); 
void tTaskDelay(uint32_t ms);
void SysTick_Handler(void);

void delay(int count)
{
	while(--count > 0); 
}

void tTaskInit(tTask* task, void(*entry)(void*), void* param, tTaskStack* stack)
{
	*(--stack) = (unsigned long)(1 << 24);
	*(--stack) = (unsigned long)entry;
	*(--stack) = (unsigned long)0x14;
	*(--stack) = (unsigned long)0x12;
	*(--stack) = (unsigned long)0x3;
	*(--stack) = (unsigned long)0x2;
	*(--stack) = (unsigned long)0x1;
	*(--stack) = (unsigned long)param;
	
	*(--stack)=(unsigned long)0x11;			    //R11
	*(--stack)=(unsigned long)0x10;			    //R10
	*(--stack)=(unsigned long)0x9;				//R9
	*(--stack)=(unsigned long)0x8;				//R8
	*(--stack)=(unsigned long)0x7;				//R7
	*(--stack)=(unsigned long)0x6;				//R6
	*(--stack)=(unsigned long)0x5;				//R5
	*(--stack)=(unsigned long)0x4;				//R4
	
	task->stack = stack;
	task->delayTicks = 0;
}


uint8_t schedLockCount;
void tTaskSchedInit(void){
	schedLockCount = 0;
}

void tTaskSchedDisable(void){
	uint32_t status = tTaskEnterCritical();
	if(schedLockCount < 255){ schedLockCount++; }
	tTaskExitCritical(status);
}

void tTaskSchedEnable(void){
	uint32_t status = tTaskEnterCritical();
	if(schedLockCount > 0){
		if(--schedLockCount == 0){
			tTaskSched();
		}
	}
	tTaskExitCritical(status);
}


void tTaskSystemTickHandler(){
	uint32_t status = tTaskEnterCritical();
	
	for(int i = 0; i < 2; i++){
		if(taskTable[i]->delayTicks > 0){
			taskTable[i]->delayTicks--;
		}
	}
	
	tTaskExitCritical(status);
	
	tTaskSched();
}

void tTaskDelay(uint32_t delay)
{
	uint32_t status = tTaskEnterCritical();
	
	currentTask->delayTicks = delay;
	
	tTaskExitCritical(status);
	tTaskSched();
}


void tSetSysTickPeriod(uint32_t ms)
{
	SysTick->LOAD = ms*SystemCoreClock/1000 -1;
	NVIC_SetPriority(SysTick_IRQn,1 << (__NVIC_PRIO_BITS -1));
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk 
					| SysTick_CTRL_TICKINT_Msk
					| SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void)
{
	 tTaskSystemTickHandler();
}

void tTaskSched()
{
	uint32_t status = tTaskEnterCritical();
	if(schedLockCount > 0) {
		tTaskExitCritical(status);
		return;
	}
	
	if(currentTask == idleTask)
	{
		if(taskTable[0]->delayTicks == 0){
			nextTask = taskTable[0];
		}else if(taskTable[1]->delayTicks == 0){
			nextTask = taskTable[1];
		}
		else  { 
			tTaskExitCritical(status);
			return; }
	}
	else  //currentTask != idleTask
	{
		if(currentTask == taskTable[0]){
			if(taskTable[1]->delayTicks == 0 ){
				nextTask = taskTable[1];
			}else if (currentTask->delayTicks != 0){
				nextTask = idleTask;
			}else { 
				tTaskExitCritical(status);
				return; }
		}
		else if(currentTask == taskTable[1]){
			if(taskTable[0]->delayTicks == 0){
				nextTask = taskTable[0];
			}else if(currentTask->delayTicks != 0){
				nextTask = idleTask;
			}else {
				tTaskExitCritical(status);
				return; }
		}
	}
	
	tTaskSwitch();
	tTaskExitCritical(status);
}


int shareCount;

void task1Entry(void *param)	//Task1 function
{
	tSetSysTickPeriod(10);
	for(;;)
	{
		int var;
		tTaskSchedDisable();
		var = shareCount;
		
		task1Flag=1;
 		tTaskDelay(1);
		var++;
		shareCount = var;
		
		tTaskSchedEnable();
		task1Flag=0;
		tTaskDelay(1);		
	}
}

void task2Entry(void *param)	//Task2 function
{
	for(;;)
	{		
		tTaskSchedDisable();
		shareCount++;
		tTaskSchedEnable();
		task2Flag=1;
		tTaskDelay(1);
		task2Flag=0;
		tTaskDelay(1);
	}
}


void idleTaskEntry(void* param){
	for(;;){}
}



int main(void)
{
	tTaskSchedInit();
	tTaskInit(&tTask1,task1Entry,(void*)0x1111111,&task1Env[1024]);
	tTaskInit(&tTask2,task2Entry,(void*)0x2222222,&task2Env[1024]);
	
	taskTable[0] = &tTask1;
	taskTable[1] = &tTask2;
	
	tTaskInit(&tTaskIdle,idleTaskEntry,(void*)0,&idleTaskEnv[1024]);
	idleTask = &tTaskIdle;
	
	nextTask = taskTable[0];
	
	tTaskRunFirst();
	
	return 0;
}




