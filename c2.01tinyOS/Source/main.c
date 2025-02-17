#include "tinyOS.h"

#define NVIC_INIT_CTRL			0xE000ED04	//The address of NVIC_INIT_CTRL register
#define NVIC_INIT_PENDSVSET		0x10000000	//The value to be written to the NVIC_INIT_CTRL

#define NVIC_INIT_SYSPRI2		0xE000ED22	//The address of NVIC_INIT_SYSPRI2 register
#define NVIC_INIT_PENDSV_PRI	0x000000FF	//The value to be written to the NVIC_INIT_PENDSV_PRI

#define MEM8(addr)		*(volatile unsigned char *)addr
#define MEM32(addr)		*(volatile unsigned long *)addr

typedef struct _BlockType_t{
	unsigned long * stackPtr;
}BlockType_t;

void delay(int count);
void triggerPendSVC(void);

int flag;
unsigned long stackBuffer[1024];
BlockType_t* blockPtr;
BlockType_t block;

tTask tTask1;
tTask tTask2;
tTaskStack task1Env[1024];
tTaskStack task2Env[1024];

void tTaskInit(tTask* task, void(*entry)(void*), void* param, tTaskStack* stack)
{
	task->stack = stack;
}



void task1(void* param){
	for(;;){}
}

void task2(void* param){
	for(;;){}
}

int main(void)
{
	block.stackPtr = &stackBuffer[1024];
	blockPtr = &block;
	
	tTaskInit(&tTask1,task1,(void*)0x1111111,&task1Env[1024]);
	tTaskInit(&tTask2,task2,(void*)0x2222222,&task2Env[1024]);
	
	for(;;)
	{
		flag = 1;
		delay(100);
		flag = 0;
		delay(100);		
		triggerPendSVC();
	}
}


void delay(int count)
{
	while(--count > 0); 
}

void triggerPendSVC(void)	//PendSVC trigger function
{
	MEM8(NVIC_INIT_SYSPRI2)=NVIC_INIT_PENDSV_PRI;		//Set the priority of PendSV
	MEM32(NVIC_INIT_CTRL)=NVIC_INIT_PENDSVSET;			//Directly trigger PendSV
}

