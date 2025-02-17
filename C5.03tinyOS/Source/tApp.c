#include "tinyOS.h"


tTask tTask1;													//Task1 structure
tTask tTask2;													//Task2 structure
tTask tTask3;													//Task2 structure
tTask tTask4;

tTaskStack task1Env[1024];						//Task1 stack
tTaskStack task2Env[1024];						//Task2 stack
tTaskStack task3Env[1024];						//Task3 stack
tTaskStack task4Env[1024];		

//定义两个结构体，表示事件类型		
tEvent eventWaitNormal;	

int task1Flag;
void task1Entry(void *param)					//Task1 function
{
	tSetSysTickPeriod(10);						//Timing cycle is 10ms	
	
	tEventInit(&eventWaitNormal, tEventTypeUnknown);
	for(;;)
	{	
		uint32_t count = tEventWaitCount(&eventWaitNormal);
		uint32_t wakeUpCount = tEventRemoveAll(&eventWaitNormal,  (void *)0, 0);
		if(wakeUpCount > 0)
		{
			tTaskSched();
			count = tEventWaitCount(&eventWaitNormal); 
		}
		
		task1Flag=0;
		tTaskDelay(1);	
		task1Flag=1;
		tTaskDelay(1);
	}
}


int task2Flag;									//Task2 flag
void task2Entry(void *param)					//Task2 function
{
	for(;;)
	{	
		tEventWait(&eventWaitNormal, currentTask, (void *)0, 0, 0);
		tTaskSched();
		
		task2Flag=0;
		tTaskDelay(1);
		task2Flag=1;
		tTaskDelay(1);
	}
}


int task3Flag;									//Task3 flag
void task3Entry(void *param)					//Task3 function
{
	
	for(;;)
	{	
		tEventWait(&eventWaitNormal, currentTask, (void *)0, 0, 0);
		tTaskSched();
		
		task3Flag=0;
		tTaskDelay(1);
		task3Flag=1;
		tTaskDelay(1);
	}
}


int task4Flag;									//Task3 flag
void task4Entry(void *param)					//Task3 function
{
	for(;;)
	{	
		tEventWait(&eventWaitNormal, currentTask, (void *)0, 0, 0);
		tTaskSched();	
		
		task4Flag=0;
		tTaskDelay(1);
		task4Flag=1;
		tTaskDelay(1);
	}
}


void tAppInit(void)										//Application initial function
{
	tTaskInit(&tTask1, task1Entry, (void *)0x11111111, 0, &task1Env[1024]);  //Task1 initiate
	tTaskInit(&tTask2, task2Entry, (void *)0x22222222, 1, &task2Env[1024]);  //Task2 initiate
	tTaskInit(&tTask3, task3Entry, (void *)0x33333333, 1, &task3Env[1024]);  //Task3 initiate
	tTaskInit(&tTask4, task4Entry, (void *)0x44444444, 1, &task4Env[1024]);  //Task3 initiate

}
