#include "tLib.h"

void tNodeInit(tNode* node)
{
	node->preNode = node;
	node->nextNode = node;
}

#define firstNode headNode.nextNode
#define lastNode headNode.preNode

void tListInit(tList *list)													//The list initial function
{
	list->firstNode = &(list->headNode);
	list->lastNode  = &(list->headNode);
	list->nodeCount=0;
}

uint32_t tListCount(tList* list)
{
	return list->nodeCount;
}

tNode* tListFirst(tList* list)
{
	tNode* node = (tNode*)0;
	if(list->nodeCount != 0){
		node = list->firstNode;
	}
	return node;
}

tNode* tListLast(tList* list)
{
	tNode* node = (tNode*)0;
	if(list->nodeCount != 0){
		node = list->lastNode;
	}
	return node;
}

tNode* tListPre(tList* list, tNode* node)
{
	if(node->preNode == node)
	{
		return (tNode*)0;
	}
	else
	{
		return node->preNode;
	}
}

tNode* tListNext(tList* list, tNode* node)
{
	if(node->nextNode == node)
	{
		return (tNode*)0;
	}
	else
	{
		return node->nextNode;
	}
}

void tListRemoveAll(tList* list)
{
	uint32_t count;
	tNode* nextNode = list->firstNode;
	for(count = list->nodeCount; count != 0; count--)
	{
		tNode* currentNode = nextNode;
		nextNode = currentNode->nextNode;
		//tNodeInit(currentNode);
		currentNode->preNode  = currentNode;
		currentNode->nextNode = currentNode;
	}
	//tListInit(tList *list)													
	list->firstNode = &(list->headNode);
	list->lastNode  = &(list->headNode);
	list->nodeCount=0;
}


void tListAddFirst(tList* list,tNode* node)
{
	node->preNode = list->firstNode->preNode; //node->preNode = list->headNode->nextNode;
	node->nextNode = list->firstNode;
	
	//原来头结点需要指向这个新的头结点
	list->firstNode->preNode = node;
	list->firstNode = node;  
	list->nodeCount++;
}

void tListAddLast(tList* list,tNode* node)
{
	node->nextNode = &(list->headNode); //node->preNode = list->headNode->nextNode;
	node->preNode = list->lastNode;
	
	//原来头结点需要指向这个新的头结点
	list->lastNode->nextNode = node;
	list->lastNode = node;  
	list->nodeCount++;
}

tNode* tListRemoveFirst(tList* list)
{
	tNode* node = (tNode*)0;
	if(list->nodeCount != 0)
	{
		node = list->firstNode;
		
		node->nextNode->preNode = &(list->headNode);
		list->firstNode = node->nextNode;
		list->nodeCount--;
	}
	return node;
}

void tListInsertAfter(tList* list, tNode* nodeAfter,tNode* nodeToInsert)
{
	nodeToInsert->nextNode = nodeAfter->nextNode;
	nodeToInsert->preNode = nodeAfter;
	
	nodeAfter->nextNode = nodeToInsert;
	nodeAfter->nextNode->preNode = nodeToInsert;
	
	list->nodeCount++;
}

void tListRemove(tList* list,tNode* node)
{
	node->preNode->nextNode = node->nextNode;
	node->nextNode->preNode = node->preNode;
	list->nodeCount--;
}

