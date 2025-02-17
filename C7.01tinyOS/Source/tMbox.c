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
