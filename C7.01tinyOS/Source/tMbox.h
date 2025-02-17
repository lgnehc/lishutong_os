#ifndef TMBOX_H
#define TMBOX_H

#include "tEvent.h"

typedef struct _tMbox
{
	tEvent event;
	uint32_t count;
	uint32_t maxCount;
	uint32_t read;
	uint32_t write;
	void** msgBuffer;
}tMbox;

void tMboxInit(tMbox* mbox,void** msgBuffer,uint32_t maxCount);

#endif
