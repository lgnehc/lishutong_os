#include "tinyOS.h"

void tMboxInit(tMbox* mbox,void** msgBuffer,uint32_t maxCount)
{
	tEventInit(&(mbox->event),tEventTypeMbox);
	mbox->msgBuffer = msgBuffer;  					//存放的mbox消息的指针
	mbox->read  = 0;
	mbox->write = 0;
	mbox->count = 0;
	mbox->maxCount = maxCount;						
}


//获取邮箱信息
uint32_t tMboxWait(tMbox* mbox,void** msg,uint32_t waitTicks)
{
	uint32_t status = tTaskEnterCritical();
	//如果有消息，那么读取出来
	if(mbox->count > 0)
	{	
		mbox->count--;
		*msg = mbox->msgBuffer[mbox->read++];		
		if(mbox->read >= mbox->maxCount)
		{
			mbox->read = 0;
		}
		tTaskExitCritical(status); 
		return tErrorNoError;
	}
	else
	{
		//没有消息，加入事件等待队列
		tEventWait(&mbox->event,currentTask,(void*)0,tEventTypeMbox,waitTicks);
		tTaskExitCritical(status); 
		tTaskSched();
		// 当切换回来时，从tTask中取出获得的消息
		*msg = currentTask->eventMsg;
		return currentTask->waitEventResult;
	}
}


uint32_t tMboxNoWaitGet(tMbox* mbox,void** msg)
{
	uint32_t status = tTaskEnterCritical();
	//如果有消息，那么读取出来
	if(mbox->count > 0)
	{	
		mbox->count--;
		*msg = mbox->msgBuffer[mbox->read++];		
		if(mbox->read >= mbox->maxCount)
		{
			mbox->read = 0;
		}
		tTaskExitCritical(status); 
		return tErrorNoError;
	}
	else
	{
		tTaskExitCritical(status);
    	return tErrorResourceUnavaliable;
	}
}



//邮箱信息通知，如果邮箱等待队列有任务，那么唤起第一个任务，没有在等待那么将消息msg写入
uint32_t tMboxNotify(tMbox* mbox,void* msg,uint32_t notifyOption)
{
	uint32_t status = tTaskEnterCritical();
	//有任务在等待信息
	if(tEventWaitCount(&(mbox->event)) > 0)
	{
		tTask* task = tEventWakeUp(&mbox->event,(void*)msg,tErrorNoError);
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}
	}
	else  //没有任务在等待信息，将消息插入到缓冲区中
	{
		//消息已满，无法插，直接退出
		if(mbox->count >= mbox->maxCount)
		{
			tTaskExitCritical(status);
			return tErrorResourceFull;
		}
		//信息从前面插入
		if(notifyOption & tMboxSendFront)
		{
			//第一个信息刚好在0号未知，那么往前插入到尾部
			if(mbox->read <= 0)
			{
				mbox->read = mbox->maxCount -1;
			}
			else
			{
				mbox->read--;
			}
			//更新信息首地址
			mbox->msgBuffer[mbox->read] = msg;
		}
		else //信息从后面插入，后面插入修改write指针
		{
			mbox->msgBuffer[mbox->write++] = msg;
			if(mbox->write >= mbox->maxCount)
			{
				mbox->write = 0;
			}
		}
		mbox->count++;
	}
	
	tTaskExitCritical(status);
	return tErrorNoError;
}


