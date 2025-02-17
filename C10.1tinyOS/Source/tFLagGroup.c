#include "tinyOS.h"
#include "tFlagGroup.h"


void tFlagGroupInit (tFlagGroup * flagGroup, uint32_t flags)
{
	tEventInit(&flagGroup->event,tEventTypeFlagGroup);
	flagGroup->flags = flags;
}


static uint32_t tFlagGroupCheckAndConsume(tFlagGroup* flagGroup,uint32_t type, uint32_t * flags)
{
	uint32_t srcFlags = *flags;
	uint32_t isSet  = type & TFLAGGROUP_SET;
	uint32_t isAll = type & TFLAGGROUP_ALL;
	uint32_t isConsume = type & TFLAGGROUP_CONSUME;
	// flagGroup->flags & srcFlags：计算出哪些位为1
	// ~flagGroup->flags & srcFlags:计算出哪位为0
	uint32_t calcFlag = isSet ? (flagGroup->flags & srcFlags) : (~flagGroup->flags & srcFlags);
	
	if( ( (isAll != 0) && (calcFlag == srcFlags) )  || ( (isAll == 0) && (calcFlag != 0) ) )
	{
		if(isConsume)
		{
			if(isSet)
			{
				// 清除为1的标志位，变成0
				flagGroup->flags &= ~srcFlags;
			}
			else
			{
				// 清除为0的标志位，变成1
				flagGroup->flags |= srcFlags;
			}
		}
		*flags = calcFlag;
		return tErrorNoError;
	}
	*flags = calcFlag;
	return tErrorResourceUnavaliable;
}	



uint32_t tFlagGroupWait(tFlagGroup* flagGroup,uint32_t waitType,uint32_t requestFlag,uint32_t* resultFlag,uint32_t waitTicks)
{
	uint32_t result;
	uint32_t flags = requestFlag;
	
	uint32_t status = tTaskEnterCritical();
	result = tFlagGroupCheckAndConsume(flagGroup,waitType,&flags);
	//插入队列
	if(result != tErrorNoError)
	{
		currentTask->waitFlagType = waitType;
		currentTask->eventFlags = requestFlag;
		tEventWait(&flagGroup->event,currentTask,(void*)0,tEventTypeFlagGroup, waitTicks);
		tTaskExitCritical(status);
		
		tTaskSched();
		*resultFlag = currentTask->eventFlags;
		result = currentTask->waitEventResult;
	}
	else
	{
		*resultFlag = flags;
        tTaskExitCritical(status);
	}
	return result;
}



uint32_t tFlagGroupNoWaitGet(tFlagGroup* flagGroup,uint32_t waitType,uint32_t requestFlag,uint32_t* resultFlag)
{
	uint32_t flags = requestFlag;
	uint32_t status = tTaskEnterCritical();
	uint32_t result = tFlagGroupCheckAndConsume(flagGroup,waitType,&flags);
	tTaskExitCritical(status);
	*resultFlag = flags;
	return tErrorNoError;
}



void tFlagGroupNotify (tFlagGroup * flagGroup, uint8_t isSet, uint32_t flags)
{
	tList* waitList;
	tNode* node;
	tNode* nextNode;
	uint8_t sched = 0;
	
	uint32_t status = tTaskEnterCritical();
	if(isSet) { flagGroup->flags |= flags; }
	else      { flagGroup->flags &= ~flags; }
	
	waitList = &flagGroup->event.waitList;
	for(node = waitList->headNode.nextNode;node != &(waitList->headNode);node = nextNode)
	{
		uint32_t result;
		tTask* task = tNodeParent(node,tTask,linkNode);
		uint32_t flags = task->eventFlags;
		nextNode = node->nextNode;
		
		result = tFlagGroupCheckAndConsume(flagGroup,task->waitFlagType,&flags);
		if(result == tErrorNoError)
		{
			task->eventFlags = flags;
			tEventWakeUpTask(&flagGroup->event,task,(void*)0,tErrorNoError);
			sched = 1;
		}
		
	}
	if(sched)
	{
		tTaskSched();
	}
	
	 tTaskExitCritical(status);
}


void tFlagGroupGetInfo (tFlagGroup * flagGroup, tFlagGroupInfo * info)
{
    uint32_t status = tTaskEnterCritical();
    info->flags = flagGroup->flags;
    info->taskCount = tEventWaitCount(&flagGroup->event);

    tTaskExitCritical(status);
}

uint32_t tFlagGroupDestroy (tFlagGroup * flagGroup)
{
    uint32_t status = tTaskEnterCritical();
    uint32_t count = tEventRemoveAll(&flagGroup->event, (void *)0, tErrorDel);
    tTaskExitCritical(status);
    if (count > 0)
    {
        tTaskSched();
    }
    return count;
}


