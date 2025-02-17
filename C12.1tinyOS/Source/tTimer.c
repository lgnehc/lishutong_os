#include "tinyOS.h"
#include "tTimer.h"

static tList tTimerHardList;
static tList tTimerSoftList;
static tSem tTimerProtectSem;
static tSem tTimerTickSem;


void tTimerInit (tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
                 void (*timerFunc) (void * arg), void * arg, uint32_t config)
{
    tNodeInit(&timer->linkNode);
    timer->startDelayTicks = delayTicks;
    timer->durationTicks = durationTicks;
    timer->timerFunc = timerFunc;
    timer->arg = arg;
    timer->config = config;

    // 如果初始启动延时为0，则使用周期值
    if (delayTicks == 0)
    {
        timer->delayTicks = durationTicks;
    }
    else
    {
        timer->delayTicks = timer->startDelayTicks;
    }
    timer->state = tTimerCreated;
}




void tTimerStart (tTimer * timer)
{
    switch (timer->state)
    {
        case tTimerCreated:
        case tTimerStopped:
            timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
            timer->state = tTimerStarted;

            // 根据定时器类型加入相应的定时器列表
            if (timer->config & TIMER_CONFIG_TYPE_HARD)
            {
                // 硬定时器，在时钟节拍中断中处理，所以使用critical来防护
                uint32_t status = tTaskEnterCritical();

                // 加入硬定时器列表
                tListAddLast(&tTimerHardList, &timer->linkNode);

                tTaskExitCritical(status);
            }
            else
            {
                // 软定时器，先获取信号量。以处理此时定时器任务此时同时在访问软定时器列表导致的冲突问题
                tSemWait(&tTimerProtectSem, 0);
                tListAddLast(&tTimerSoftList, &timer->linkNode);
                tSemNotify(&tTimerProtectSem);
            }
            break;
        default:
            break;
    }
}

/**********************************************************************************************************
** Function name        :   tTimerStop
** Descriptions         :   终止定时器
** parameters           :   timer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void tTimerStop (tTimer * timer)
{
    switch (timer->state)
    {
        case tTimerStarted:
        case tTimerRunning:
            // 如果已经启动，判断定时器类型，然后从相应的延时列表中移除
            if (timer->config & TIMER_CONFIG_TYPE_HARD)
            {
                // 硬定时器，在时钟节拍中断中处理，所以使用critical来防护
                uint32_t status = tTaskEnterCritical();

                // 从硬定时器列表中移除
                tListRemoveNode(&tTimerHardList, &timer->linkNode);

                tTaskExitCritical(status);
            }
            else
            {
                // 软定时器，先获取信号量。以处理此时定时器任务此时同时在访问软定时器列表导致的冲突问题
                tSemWait(&tTimerProtectSem, 0);
                tListRemoveNode(&tTimerSoftList, &timer->linkNode);
                tSemNotify(&tTimerProtectSem);
            }
            timer->state = tTimerStopped;
            break;
        default:
            break;
    }
}



void tTimerDestroy (tTimer * timer)
{
    tTimerStop(timer);
    timer->state = tTimerDestroyed;
}

/**********************************************************************************************************
** Function name        :   tTimerGetInfo
** Descriptions         :   查询状态信息
** parameters           :   timer 查询的定时器
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void tTimerGetInfo (tTimer * timer, tTimerInfo * info)
{
    uint32_t status = tTaskEnterCritical();

    info->startDelayTicks = timer->startDelayTicks;
    info->durationTicks = timer->durationTicks;
    info->timerFunc = timer->timerFunc;
    info->arg = timer->arg;
    info->config = timer->config;
    info->state = timer->state;

    tTaskExitCritical(status);
}






static void tTimerCallFuncList (tList * timerList)
{
    tNode * node;
    
    // 检查所有任务的delayTicks数，如果不0的话，减1。
    for (node = timerList->headNode.nextNode; node != &(timerList->headNode); node = node->nextNode)
    {
        tTimer * timer = tNodeParent(node, tTimer, linkNode);

        // 如果延时已到，则调用定时器处理函数
        if ((timer->delayTicks == 0) || (--timer->delayTicks == 0))
        {
            // 切换为正在运行状态
            timer->state = tTimerRunning;

            // 调用定时器处理函数
            timer->timerFunc(timer->arg);

            // 切换为已经启动状态
            timer->state = tTimerStarted;

            if (timer->durationTicks > 0)
            {
                // 如果是周期性的，则重复延时计数值
                timer->delayTicks = timer->durationTicks;
            }
            else
            {
                // 否则，是一次性计数器，中止定时器
                tListRemoveNode(timerList, &timer->linkNode);
                timer->state = tTimerStopped;
            }
        }
    }
}


void tTimerModuleTickNotify (void)
{
    uint32_t status = tTaskEnterCritical();

    // 处理硬定时器列表
    tTimerCallFuncList(&tTimerHardList);

    tTaskExitCritical(status);

    // 通知软定时器节拍变化
    tSemNotify(&tTimerTickSem);
}



static tTask tTimeTask;
static tTaskStack tTimerTaskStack[TINYOS_TIMER_STACK_SIZE];

static void tTimerSoftTask (void * param)
{
    for (;;)
    {
        
        tSemWait(&tTimerTickSem, 0);
        tSemWait(&tTimerProtectSem, 0);
        tTimerCallFuncList(&tTimerSoftList);
        tSemNotify(&tTimerProtectSem);
    }
}




void tTimerModuleInit (void)
{
    tListInit(&tTimerHardList);
    tListInit(&tTimerSoftList);
    tSemInit(&tTimerProtectSem, 1, 1);
    tSemInit(&tTimerTickSem, 0, 0);

#if TINYOS_TIMERTASK_PRIO >= (TINYOS_PRO_COUNT - 1)
    #error "The proprity of timer task must be greater then (TINYOS_PRO_COUNT - 1)"
#endif
    tTaskInit(&tTimeTask, tTimerSoftTask, (void *)0,
        TINYOS_TIMERTASK_PRIO, tTimerTaskStack, TINYOS_TIMER_STACK_SIZE);
}
