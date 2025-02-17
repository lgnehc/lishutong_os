#include "tinyOS.h"

void tSemInit(tSem* sem,uint32_t startCount,uint32_t maxCount)
{
	tEventInit(&(sem->event),tEventTypeSem);
	sem->maxCount = maxCount;
	if(sem->maxCount == 0)
	{
		sem->maxCount = 0;
	}
	else
	{
		sem->count = (startCount < maxCount)? startCount:maxCount;
	}
}

//获取信号量，任务需要资源，先调用这个函数判断是否有资源，资源没有时进入信号量等待
uint32_t tSemWait(tSem* sem,uint32_t timeout)
{
	uint32_t status=tTaskEnterCritical();
	//有资源直接减1
	if(sem->count > 0)
	{
		sem->count --;
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	//没有资源就加入sem信号量中的等待队列
	else
	{
		tEventWait(&(sem->event),currentTask,(void*)0,tEventTypeSem,timeout);
		tTaskExitCritical(status);
		
		tTaskSched();
		return currentTask->waitEventResult;
	}
	
}


uint32_t tSemNoWaitGet(tSem* sem)
{
	uint32_t status=tTaskEnterCritical();
	if(sem->count > 0)
	{
		sem->count --;
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else
	{ 
		tTaskExitCritical(status); 
		return tErrorResourceUnavaliable;
	}
}



//事件通知，
void tSemNotify(tSem* sem)
{
	tTask* task;
	uint32_t status=tTaskEnterCritical();
	//如果sem事件控制块中有任务在等待,那么唤起第一个加入任务队列
	if(tEventWaitCount(&(sem->event))  > 0 )
	{
		task = tEventWakeUp(&(sem->event),(void*)0,tErrorNoError);
		if(task->prio < currentTask->prio)
		{  tTaskSched(); }
	}
	//如果sem事件控制块中没有任务
	else 
	{
		sem->count++;   //为什么++
		//确保计数值上限 
		if( ((sem->maxCount)!=0) && (sem->count) > (sem->maxCount) )
		{
			sem->count = sem->maxCount;
		}	
	}
	
	tTaskExitCritical(status);
}


void tSemGetInfo(tSem* sem,tSemInfo* Info)
{
	uint32_t status=tTaskEnterCritical();
	
	Info->count = sem->count;
	Info->maxCount = sem->maxCount;
	Info->taskCount = tEventWaitCount(&(sem->event));
	
	tTaskExitCritical(status);
	
}

uint32_t tSemDestroy(tSem* sem)
{
	uint32_t status=tTaskEnterCritical();
	
	uint32_t count = tEventRemoveAll(&(sem->event),(void*)0,tErrorDel); 
	sem->count = 0;
	
	tTaskExitCritical(status);
	
	if(count > 0)
	{
		tTaskSched();
	}
	return count;	
}

