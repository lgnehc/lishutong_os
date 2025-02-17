#include "tinyOS.h"

void tEventInit(tEvent* event, tEventType type)
{
	event->type = type;
	tListInit(&(event->waitList));
}


void tEventWait(tEvent* event, tTask* task,void* msg,
				uint32_t state,uint32_t timeout)
{
	uint32_t status = tTaskEnterCritical();
	
	task->waitEvent = event;
	task->eventMsg = msg;
    task->state |= state;
	task->waitEventResult = tErrorNoError;
	
	tTaskSchedUnRdy(task);
	tListAddLast(&(event->waitList),&(task->linkNode));
	
	if(timeout != 0 )
	{
		tTimeTaskWait(task,timeout);
	}
	
	tTaskExitCritical(status);
}

//从事件控制块中唤起第一个任务
tTask* tEventWakeUp(tEvent* event,void* msg,uint32_t result)
{
	tNode* node = (tNode*)0;
	tTask* task = (tTask*)0;
	
	uint32_t status = tTaskEnterCritical();
	//如果是空队列，那么返回0
	node = tListRemoveFirst(&(event->waitList));
	//返回不为零，代表这个event->waitList不为空,唤醒第一个任务
	//那么需要调整当前event中的task状态
	if(node != (tNode*)0)
	{
		task = tNodeParent(node,tTask,linkNode);
		task->waitEvent = (tEvent*)0;
		task->eventMsg = msg;
		task->state &= ~TINYOS_TASK_WAIT_MASK;
		task->waitEventResult = result;
		
		if(task->delayTicks != 0)
		{
			tTimeTaskWakeUp(task);
		}
		tTaskSchedRdy(task);
	}
	
	tTaskExitCritical(status);
	return task;
}


void tEventRemoveTask(tTask *task, void *msg, uint32_t result)
{
	uint32_t status=tTaskEnterCritical();
	
	tListRemoveNode(&(task->waitEvent->waitList),&(task->linkNode));
	
	task->waitEvent=(tEvent*)0;
	task->eventMsg=msg;
	task->state &= ~TINYOS_TASK_WAIT_MASK;
	task->waitEventResult=result;
	
	tTaskExitCritical(status);
}


