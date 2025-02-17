#include "tinyOS.h"

tTask tTask1;													//Task1 structure
tTask tTask2;													//Task2 structure
tTask tTask3;													
//Task2 structure
tTask tTask4;

tTaskStack task1Env[1024];						//Task1 stack
tTaskStack task2Env[1024];						//Task2 stack
tTaskStack task3Env[1024];						//Task3 stack
tTaskStack task4Env[1024];		

tMbox box1;
tMbox box2;
void* mbox1MsgBuffer[20];
void* mbox2MsgBuffer[20];
uint32_t msg[20];

int task1Flag;
void task1Entry(void *param)					//Task1 function
{
	tSetSysTickPeriod(10);						//Timing cycle is 10ms	
 	tMboxInit(&box1,(void*)mbox1MsgBuffer,20);
	for(;;)
	{	
		uint32_t i = 0;
		for(i = 0;i < 20;i++)
		{
			msg[i] = i;
			tMboxNotify(&box1,&msg[i],tMboxSendNormal);
		}
		
		tTaskDelay(100);
		
		for(i = 0;i < 20;i++)
		{
			msg[i] = i;
			tMboxNotify(&box1,&msg[i],tMboxSendFront);
		}
		tTaskDelay(100);
		
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
		void* msg;
		uint32_t err = tMboxWait(&box1,&msg,0);
		if(err == tErrorNoError)
		{
			uint32_t value = *(uint32_t *)msg;
			task2Flag = value;
			tTaskDelay(1);
		}
		
		task2Flag = 0;
		tTaskDelay(1);
		task2Flag = 1;
		tTaskDelay(1);
	}
}


int task3Flag;									//Task3 flag
void task3Entry(void *param)					//Task3 function
{
	tMboxInit(&box2,mbox2MsgBuffer,20);
	for(;;)
	{	
		void* msg;
		tMboxWait(&box2,&msg,100);
		
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
