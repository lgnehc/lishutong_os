#include "tinyOS.h"


tTask tTask1;													//Task1 structure
tTask tTask2;													//Task2 structure
tTask tTask3;													//Task2 structure
tTask tTask4;

tTaskStack task1Env[1024];						//Task1 stack
tTaskStack task2Env[1024];						//Task2 stack
tTaskStack task3Env[1024];						//Task3 stack
tTaskStack task4Env[1024];		

int task1Flag;
void task1Entry(void *param)					//Task1 function
{
	taskInfo taskinfo;
	
	tSetSysTickPeriod(10);						//Timing cycle is 10ms	
	for(;;)
	{	
		tTaskGetInfo(currentTask,&taskinfo);
		tTaskGetInfo(&tTask4,&taskinfo);
		
		task1Flag=1;
		tTaskDelay(10);
		task1Flag=0;
		tTaskDelay(10);
	}
}


int task2Flag;									//Task2 flag
void task2Entry(void *param)					//Task2 function
{
	for(;;)
	{		
		task2Flag=1;
		tTaskDelay(10);
		task2Flag=0;
		tTaskDelay(10);
	}
}


int task3Flag;									//Task3 flag
void task3Entry(void *param)					//Task3 function
{
	for(;;)
	{		
		
		task3Flag=1;
		tTaskDelay(10);
		task3Flag=0;
		tTaskDelay(10);
	}
}


int task4Flag;									//Task3 flag
void task4Entry(void *param)					//Task3 function
{
	for(;;)
	{		
		task4Flag=1;
		tTaskDelay(10);
		task4Flag=0;
		tTaskDelay(10);
	}
}


void tAppInit(void)										//Application initial function
{
	tTaskInit(&tTask1, task1Entry, (void *)0x11111111, 0, &task1Env[1024]);  //Task1 initiate
	tTaskInit(&tTask2, task2Entry, (void *)0x22222222, 1, &task2Env[1024]);  //Task2 initiate
	tTaskInit(&tTask3, task3Entry, (void *)0x33333333, 0, &task3Env[1024]);  //Task3 initiate
	tTaskInit(&tTask4, task4Entry, (void *)0x44444444, 1, &task4Env[1024]);  //Task3 initiate

}
