#include "tinyOS.h"
#include "ARMCM3.h"


int task1Flag;	//Task1 flag
int task2Flag;	//Task2 flag
int task3Flag;	//Task3 flag

tTask tTask1;
tTask tTask2;
tTask tTask3;
tTask tTaskIdle;
tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];
tTaskStack idleTaskEnv[1024];

tTask* currentTask;
tTask* nextTask;
tTask* idleTask;

tList taskTable[TINYOS_PRIO_COUNT];
tBitmap taskPrioBitmap;
tList tTaskDelayedList;	


void tTaskInit(tTask* task, void(*entry)(void*), void* param, uint32_t prio, tTaskStack* stack);
void tSetSysTickPeriod(uint32_t ms);
void tTaskSched();
void task1Entry(void *param);
void task2Entry(void *param);
void tTaskSystemTickHandler(); 
void tTaskDelay(uint32_t ms);
void SysTick_Handler(void);
void tTaskSchedInit(void);
tTask* tTaskHigestReady(void);

void tTaskDelayInit(void);
void tTimeTaskWait(tTask* task,uint32_t ticks);
void tTimeTaskWakeup(tTask* task);
void tTaskSchedRdy(tTask* task);
void tTaskSchedUnRdy(tTask* task);


void tTaskInit(tTask* task, void(*entry)(void*), void* param,uint32_t prio, tTaskStack* stack)
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
	task->prio = prio;
	task->state = TINYOS_TASK_STATE_RDY;
	tNodeInit(&(task->delayNode));
	tNodeInit(&(task->linkNode));	
	task->slice=TINYOS_SLICE_MAX;
	
	tListAddFirst(&taskTable[prio], &(task->linkNode));
	tBitmapSet(&taskPrioBitmap,prio);
}

tTask* tTaskHigestReady(void){
	uint32_t higestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
	tNode* node = tListFirst(&taskTable[higestPrio]);
	return tNodeParent(node, tTask, linkNode);
}

uint8_t schedLockCount;

void tTaskSchedInit(void){
	schedLockCount = 0;
	tBitmapInit(&taskPrioBitmap);
	for(int i=0; i<TINYOS_PRIO_COUNT; i++)
	{
		tListInit(&taskTable[i]);
	}
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


void tTaskSystemTickHandler()
{
	tNode *node;
	tTask *task;
	uint32_t status=tTaskEnterCritical();
	
	for(node=tTaskDelayedList.firstNode; node!= &(tTaskDelayedList.headNode); node=node->nextNode)
	{
		task=tNodeParent(node, tTask, delayNode);
		if(--task->delayTicks==0)
		{
			tTimeTaskWakeup(task);
			tTaskSchedRdy(task);
		}
	}
	
	if(--(currentTask->slice)==0)			//Time slice round-robin
	{
		if(tListCount(&taskTable[currentTask->prio])>0)
		{
			tListRemoveFirst(&taskTable[currentTask->prio]);
			tListAddLast(&taskTable[currentTask->prio], &(currentTask->linkNode));
			currentTask->slice=TINYOS_SLICE_MAX;
		}
	}
	
	tTaskExitCritical(status);
	
	tTaskSched();
}

void tTaskDelay(uint32_t delay)
{
	uint32_t status = tTaskEnterCritical();
	
	tTimeTaskWait(currentTask, delay);
	
	tTaskSchedUnRdy(currentTask);
	
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
	tTask* tempTask;
	if(schedLockCount > 0) {
		tTaskExitCritical(status);
		return;
	}
	
	tempTask = tTaskHigestReady();
	if (tempTask != currentTask){
		nextTask = tempTask;
		tTaskSwitch();
	}
	
	tTaskExitCritical(status);
}


void tTaskDelayInit(void)
{
	tListInit(&tTaskDelayedList);
}


void tTimeTaskWait(tTask* task,uint32_t ticks )
{
	task->delayTicks = ticks;
	tListAddLast(&tTaskDelayedList,&(task->delayNode));
	task->state |= TINYOS_TASK_STATE_DELAY;
}


void tTimeTaskWakeup(tTask* task)
{
	tListRemoveNode(&tTaskDelayedList, &(task->delayNode));
	task->state &= ~TINYOS_TASK_STATE_DELAY;
}

void tTaskSchedRdy(tTask* task)
{
	tListAddFirst(&taskTable[task->prio], &(task->linkNode));
	tBitmapSet(&taskPrioBitmap,task->prio);
}

void tTaskSchedUnRdy(tTask* task)
{
	tListRemoveNode(&taskTable[task->prio], &(task->linkNode));
	
	if(tListCount(&taskTable[task->prio])==0)
	{
		tBitmapClear(&taskPrioBitmap, task->prio);
	}
}

void delay(void)														//Task delay function2
{
	int i;
	for(i=0; i<0xFF; i++){}
}


void task1Entry(void *param)	//Task1 function
{	
 	tSetSysTickPeriod(10);	
	for(;;)
	{
		task1Flag=1;
 		tTaskDelay(10);
		task1Flag=0;
		tTaskDelay(10);		
	}
}

void task2Entry(void *param)						//Task2 function
{
	for(;;)
	{		
		task2Flag=1;
		delay();
		task2Flag=0;
		delay();
	}
}


void task3Entry(void *param)						//Task3 function
{
	for(;;)
	{		
		task3Flag=1;
		delay();
		task3Flag=0;
		delay();
	}
}

void idleTaskEntry(void* param){
	for(;;){}
}



int main(void)
{
	tTaskSchedInit();
	tTaskDelayInit();
	tTaskInit(&tTask1,task1Entry,(void*)0x1111111,0,&task1Env[1024]);
	tTaskInit(&tTask2,task2Entry,(void*)0x2222222,1,&task2Env[1024]);
	tTaskInit(&tTask3,task3Entry,(void*)0x3333333,1,&task3Env[1024]);
	tTaskInit(&tTaskIdle,idleTaskEntry,(void*)0,TINYOS_PRIO_COUNT -1,&idleTaskEnv[1024]);
	
	idleTask = &tTaskIdle;
	
	nextTask = tTaskHigestReady();
	tTaskRunFirst();
	
	return 0;
}




