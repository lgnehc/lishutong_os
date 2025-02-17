#ifndef tTASK_H
#define tTASK_H

#include "stdint.h"
#include "tLib.h"

#define TINYOS_TASK_STATUS_RDY              0			//Task is not delayed and ready
#define TINYOS_TASK_STATUS_DESTORYED      (1<<1)
#define TINYOS_TASK_STATUS_DELAYED		  (1<<2)		//Task is delayed 
#define TINYOS_TASK_STATUS_SUSPENDED	  (1<<3)		//Task is suspeded 
#define TINYOS_TASK_WAIT_MASK		 (0xFF << 16)

struct _tEvent;
typedef uint32_t tTaskStack;		//Task stack data type
typedef struct _tTask					//Task structure data type
{
	uint32_t *stack;							//Task stack pointer
	tNode linkNode;								//Through this node, the task can be inserted to the taskTable
	uint32_t delayTicks;					//Task delay counter
	tNode delayNode;							//Through this node, the task can be inserted to the task delayed list 
	uint32_t prio;								//Task priority
	uint32_t state;								//Present the delay state of the task
	uint32_t slice;								//Time slice counter
	uint32_t suspendCount;				//Task suspended conunter
	
	void(* clean)(void* param);
	void* cleanParam;
	uint8_t requestDeleteFlag;
	
	struct _tEvent *waitEvent;
	//tEvent *waitEvent;
	void *eventMsg;
	uint32_t waitEventResult;	
}tTask;


typedef struct _taskInfo{
	
	uint32_t delayTicks;					//Task delay counter
	uint32_t prio;								//Task priority
	uint32_t state;								//Present the delay state of the task
	uint32_t slice;								//Time slice counter
	uint32_t suspendCount;				//Task suspended conunter
}taskInfo;


void tTaskInit(tTask *task, void(*entry)(void *), void *param,
								uint32_t prio, tTaskStack *stack);								//Task initial function																							//Application initial function
void tTaskSuspend(tTask *task);																		//To suspend the task
void tTaskWakeUp(tTask *task);			
														//To wake up the task
//任务删除相关函数
void tTaskSchedRemove(tTask *task);
void tTimeTaskRemove(tTask* task);
void tTaskSetCleanCallFunc(tTask* task, void (*clean)(void * param), void* param);
void tTaskForceDelete(tTask* task);
void tTaskRequestDelete(tTask* task);
uint8_t tTaskIsRequestDelete(void);
void tTaskDeleteSelf(void);	

//任务状态信息查询
void tTaskGetInfo(tTask* task, taskInfo* taskinfo);


#endif
