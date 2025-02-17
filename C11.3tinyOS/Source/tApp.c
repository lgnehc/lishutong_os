#include "tinyOS.h"

tTask tTask1;													//Task1 structure
tTask tTask2;													//Task2 structure
tTask tTask3;													
tTask tTask4;

tTaskStack task1Env[1024];						//Task1 stack
tTaskStack task2Env[1024];						//Task2 stack
tTaskStack task3Env[1024];						//Task3 stack
tTaskStack task4Env[1024];		

tTimer timer1;
tTimer timer2;
tTimer timer3;
uint32_t bit1 = 0;
uint32_t bit2 = 0;
uint32_t bit3 = 0;

uint32_t counter = 0;
void timerFunc (void * arg)
{
    // 简单的将最低位取反，输出高低翻转的信号
    uint32_t * ptrBit = (uint32_t *)arg;
    *ptrBit ^= 0x1;
}

int task1Flag;
tTimerInfo timerInfo;
void task1Entry (void * param)
{
    uint32_t destroyed = 0;

    tSetSysTickPeriod(10);

    // 定时器1：100个tick后启动，以后每10个tick启动一次
    tTimerInit(&timer1, 100, 10, timerFunc, (void *)&bit1, TIMER_CONFIG_TYPE_HARD);
    tTimerStart(&timer1);

    // 定时器2：200个tick后启动，以后每20个tick启动一次
    tTimerInit(&timer2, 200, 20, timerFunc, (void *)&bit2, TIMER_CONFIG_TYPE_HARD);
    tTimerStart(&timer2);

    // 定时器1：300个tick后启动，启动之后关闭
    tTimerInit(&timer3, 300, 0, timerFunc, (void *)&bit3, TIMER_CONFIG_TYPE_HARD);
    tTimerStart(&timer3);
    for (;;)
    {
        task1Flag = 1;
        tTaskDelay(1);
        task1Flag = 0;
        tTaskDelay(1);

        // 200个tick后，手动销毁定时器1
        if (destroyed == 0)
        {
            tTaskDelay(200);
            tTimerDestroy(&timer1);
            destroyed = 1;
        }

        // 获取定时器2的信息
        tTimerGetInfo(&timer2, &timerInfo);
    }
}


int task2Flag;									//Task2 flag
void task2Entry(void *param)					//Task2 function
{	

	for(;;)
	{		
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
