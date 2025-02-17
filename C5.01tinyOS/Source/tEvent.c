#include "tinyOS.h"


void tEventInit(tEvent* event, tEventType type)
{
	event->type = type;
	tListInit(&event->waitList);
}
