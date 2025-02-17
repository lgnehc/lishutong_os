#include "tSem.h"

void tSemInit(tSem* sem,uint32_t startCount,uint32_t maxCount)
{
	tEventInit(&(sem->event),tEventTypeSem);
	sem->maxCount = maxCount;
	if(sem->maxCount == 0)
	{
		sem->maxCount = 0;
	}
	else
	{
		sem->count = (startCount < maxCount)? startCount:maxCount;
	}
}
