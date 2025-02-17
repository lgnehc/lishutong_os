#include "tinyOS.h"
#include "ARMCM3.h"

tTask *currentTask;											
tTask *nextTask;											
tTask *idleTask;											
tList taskTable[TINYOS_PRO_COUNT];							
tTask tTaskIdle;											
tTaskStack idleTaskEnv[TINYOS_IDLETASK_STACK_SIZE];			

uint8_t schedLockCount;								
tBitmap taskPrioBitmap;								
tList tTaskDelayedList;	


uint32_t idleCount;
uint32_t idleMaxCount;
uint32_t tickCount;

static float cpuUsage;                      
static uint32_t enableCpuUsageStat;  

static void initCpuUsageStat (void);
 void checkCpuUsage (void);
static void cpuUsageSyncWithSysTick (void);

static void initCpuUsageStat (void)
{
    idleCount = 0;
    idleMaxCount = 0;
    cpuUsage = 0.0f;
    enableCpuUsageStat = 0;
}


 void checkCpuUsage (void)
{
    // 与空闲任务的cpu统计同步
    if (enableCpuUsageStat == 0)
    {
        enableCpuUsageStat = 1;
        tickCount = 0;
        return;
    }

    if (tickCount == TICKS_PER_SEC)
    {
        // 统计最初1s内的最大计数值
        idleMaxCount = idleCount;
        idleCount = 0;

        // 计数完毕，开启调度器，允许切换到其它任务
        tTaskSchedEnable();
    }
    else if (tickCount % TICKS_PER_SEC == 0)
    {
        // 之后每隔1s统计一次，同时计算cpu利用率
        cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
        idleCount = 0;
    }
} 



static void cpuUsageSyncWithSysTick (void)
{
    // 等待与时钟节拍同步
    while (enableCpuUsageStat == 0)
    {
        ;;
    }
}

float tCpuUsageGet (void)
{
    float usage = 0;

    uint32_t status = tTaskEnterCritical();
    usage = cpuUsage;
    tTaskExitCritical(status);

    return usage;
}



void idleTaskEntry(void *param)				
{
	tTaskSchedDisable();
	tAppInit(); 
	tTimerInitTask();
	tSetSysTickPeriod(TINYOS_SYSTICK_MS);
	cpuUsageSyncWithSysTick();
	
	for(;;)
	{
		uint32_t status=tTaskEnterCritical();
		idleCount++;
		tTaskExitCritical(status);
	}
}


int main(void)
 {
	tTaskSchedInit();
	tTaskDelayInit();																				
	tTimerModuleInit();
	initCpuUsageStat();
	
	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, TINYOS_PRO_COUNT-1, 
 						idleTaskEnv,TINYOS_IDLETASK_STACK_SIZE);						
	
	idleTask=&tTaskIdle;
	
	nextTask=tTaskHighestReady();			
	tTaskRunFirst();
	return 0;
}

