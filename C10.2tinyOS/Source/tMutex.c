#include "tMutex.h"
#include "tinyOS.h"

void tMutexInit (tMutex * mutex)
{
	tEventInit(&mutex->event,tEventTypeMutex);
	mutex->lockedCount = 0;
	mutex->owner = (void*)0;
	mutex->ownerOriginalPrio = TINYOS_PRO_COUNT;
}

uint32_t tMutexWait(tMutex* mutex,uint32_t waitTicks)
{
	uint32_t status = tTaskEnterCritical();
	//case1：当前信号量没有被锁定
	if(mutex->lockedCount <= 0)
	{
		mutex->owner = currentTask;
		mutex->ownerOriginalPrio = currentTask->prio;
		mutex->lockedCount++;
		
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	//case2:当前信号量已被锁定
	else
	{
		//case2.1:是否自己锁定
		if(mutex->owner == currentTask)
		{
			mutex->lockedCount++;
			tTaskExitCritical(status);
			return tErrorNoError;
		}
		//case2.2:不是自己锁定，优先级低的先锁定，策略是优先级翻转
		else
		{
			if(currentTask->prio < mutex->owner->prio)
			{
				tTask* owner = mutex->owner;
				//占用的任务是在ready状态，先移出就绪队列修改优先级在加入就绪队列
				if(owner->state == TINYOS_TASK_STATUS_RDY)
				{
					tTaskSchedUnRdy(owner);
					owner->prio = currentTask->prio;
					tTaskSchedRdy(owner);
				}
				else //其他状态直接修改优先级
				{
					owner->prio = currentTask->prio;
				}
			}
			//修改优先级后加入等待队列
			//也包括不需要修改优先级的情形
			tEventWait(&mutex->event,currentTask,(void*)0,tEventTypeMutex, waitTicks);
			tTaskExitCritical(status);
			
			tTaskSched();
			return currentTask->waitEventResult;
		}
	}
}


uint32_t tMutexNoWaitGet(tMutex* mutex)
{
	uint32_t status = tTaskEnterCritical();
	//case1：当前信号量没有被锁定
	if(mutex->lockedCount <= 0)
	{
		mutex->owner = currentTask;
		mutex->ownerOriginalPrio = currentTask->prio;
		mutex->lockedCount++;
		
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else
	{
		if(mutex->owner == currentTask)
		{
			mutex->lockedCount++;
			tTaskExitCritical(status);
			return tErrorNoError;
		}
		tTaskExitCritical(status);	
		return tErrorResourceUnavaliable;
	}
}


uint32_t tMutexNotify(tMutex* mutex)
{
	uint32_t status = tTaskEnterCritical();
	//case1:未被锁定，直接退出
	if(mutex->lockedCount <= 0)
	{
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	//case2.1:已经锁定，判断是否自己锁定,不是自己锁定直接退出
	if(mutex->owner != currentTask)
	{
		tTaskExitCritical(status);
        return tErrorOwner;
	}
	//case2.2:自己锁定情形--不是最后一次锁定计数减一退出
	if(--mutex->lockedCount > 0)
	{
		tTaskExitCritical(status);
        return tErrorNoError;
	}
	//case2.3:执行到最后一次锁定，优先级翻转恢复
	if(mutex->ownerOriginalPrio != mutex->owner->prio)
	{
		if(mutex->owner->state == TINYOS_TASK_STATUS_RDY)
		{
			tTaskSchedUnRdy(mutex->owner);
			currentTask->prio = mutex->ownerOriginalPrio;
			tTaskSchedRdy(mutex->owner);
		}
		else //其他状态只修改优先级
		{
			currentTask->prio = mutex->ownerOriginalPrio;
		}
	}
	
	//检查是否有任务等待
	if(tEventWaitCount(&mutex->event) > 0)
	{
		tTask* task = tEventWakeUp(&mutex->event,(void *)0, tErrorNoError);
		mutex->owner = task;
		mutex->ownerOriginalPrio = task->prio;
		mutex->lockedCount++;
		
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}
	}
	tTaskExitCritical(status);
    return tErrorNoError;
}


