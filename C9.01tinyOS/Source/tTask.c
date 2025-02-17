#include "tinyOS.h"

void tTaskInit(tTask *task, void(*entry)(void *), 
			   void *param, uint32_t prio, tTaskStack *stack)			//Task initial function
{
	//These data are pushed or popped using PSP pointer by the hardware automaticly
	*(--stack)=(unsigned long)(1<<24);		//xPSR
	*(--stack)=(unsigned long)entry;			//PC(R15)
	*(--stack)=(unsigned long)0x50C;			//R14
	*(--stack)=(unsigned long)0x12;			//R12
	*(--stack)=(unsigned long)0x3;				//R3
	*(--stack)=(unsigned long)0x2;				//R2
	*(--stack)=(unsigned long)0x1;				//R1
	*(--stack)=(unsigned long)param;			//param(R0)
	
	//These data are pushed or popped using task structure stack pointer by user manually
	*(--stack)=(unsigned long)0x11;			//R11
	*(--stack)=(unsigned long)0x10;			//R10
	*(--stack)=(unsigned long)0x9;				//R9
	*(--stack)=(unsigned long)0x8;				//R8
	*(--stack)=(unsigned long)0x7;				//R7
	*(--stack)=(unsigned long)0x6;				//R6
	*(--stack)=(unsigned long)0x5;				//R5
	*(--stack)=(unsigned long)0x4;				//R4
	
	task->stack=stack;										//Initiate the task stack pointer
	task->delayTicks=0;										//Initiate the task delay counter
	tNodeInit(&(task->delayNode));				//Initiate the task's delay node
	tNodeInit(&(task->linkNode));					//Initiate the task's link node
	task->prio=prio;											//Initiate the task priority
	task->state=TINYOS_TASK_STATUS_RDY;		//Initiate the task delay state: ready
	task->slice=TINYOS_SLICE_MAX;
	task->suspendCount=0;
	task->requestDeleteFlag = 0;
	task->clean=(void(*)(void *))0; 		//Initiate the callback function pointer
	task->cleanParam=(void *)0;
	
	tListAddFirst(&taskTable[prio], &(task->linkNode));

	tBitmapSet(&taskPrioBitmap,prio);
}

void tTaskSuspend(tTask *task)					//To suspend the task
{
	uint32_t status=tTaskEnterCritical();
	
	if(!(task->state & TINYOS_TASK_STATUS_DELAYED))			//The task isn't delayed
	{
		if(++task->suspendCount<=1)												//This is the first time to suspend the task
		{
			task->state |= TINYOS_TASK_STATUS_SUSPENDED;
			tTaskSchedUnRdy(task);
		}
		if(task==currentTask)
		{
			tTaskSched();
		}
	}
	
	tTaskExitCritical(status);
}

void tTaskWakeUp(tTask *task)						//To wake up the task
{
	uint32_t status=tTaskEnterCritical();
	
	if(task->state & TINYOS_TASK_STATUS_SUSPENDED)			//The task is suspended
	{
		if(--task->suspendCount==0)
		{
			task->state &= ~TINYOS_TASK_STATUS_SUSPENDED;
			tTaskSchedRdy(task);
			tTaskSched();
		}
	}
	
	tTaskExitCritical(status);
}


//删除任务中的延迟队列
void tTimeTaskRemove(tTask* task)
{
	tListRemoveNode(&tTaskDelayedList,&(task->delayNode));
}

//删除任务的任务列表和bitmap
void tTaskSchedRemove(tTask *task)
{
	tListRemoveNode(&taskTable[task->prio],&(task->linkNode));
	if (tListCount(&taskTable[task->prio]) == 0)
	{
		tBitmapClear(&taskPrioBitmap, task->prio); 
	}
}


void tTaskSetCleanCallFunc(tTask* task, void (*clean)(void * param), void* param)
{
	task->clean = clean;
	task->cleanParam = param;
}

void tTaskForceDelete(tTask* task)
{
	uint32_t status=tTaskEnterCritical();
	if(task->state & TINYOS_TASK_STATUS_DELAYED)
	{
		tTimeTaskRemove(task);
	}
	//不包含suspend状态，且不是delay状态,直接从任务队列删除
	else if (! (task->state & TINYOS_TASK_STATUS_SUSPENDED))
	{
		tTaskSchedRemove(task);
	}
	
	if(task->clean)
	{
		task->clean(task->cleanParam);
	}
	if(currentTask == task)  
	{ 
		tTaskSched(); 
	}
	tTaskExitCritical(status);
}

//请求删除别人
void tTaskRequestDelete(tTask* task)
{
	uint32_t status=tTaskEnterCritical();
	task->requestDeleteFlag = 1 ;
	tTaskExitCritical(status);
}


//查询自己是否被删除
uint8_t tTaskIsRequestDelete(void)
{
	uint32_t status=tTaskEnterCritical();
	uint8_t deleteFlag = currentTask->requestDeleteFlag; 
	tTaskExitCritical(status);
	return deleteFlag;
}

void tTaskDeleteSelf(void)
{
	uint32_t status=tTaskEnterCritical();
	tTaskSchedRemove(currentTask);
	if(currentTask->clean)
	{
		currentTask->clean(currentTask->cleanParam);
	}	
	tTaskSched();
	
	tTaskExitCritical(status);
}

//任务状态信息查询
void tTaskGetInfo(tTask* task, taskInfo* taskinfo)
{
	taskinfo->delayTicks=task->delayTicks;					//Task delay counter
	taskinfo->prio=task->prio;									//Task priority
	taskinfo->state=task->state;									//Present the delay state of the task
	taskinfo->slice=task->slice;									//Time slice counter
	taskinfo->suspendCount=task->suspendCount;
}
