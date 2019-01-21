#ifndef LINKEDSTACK_H
#define LINKEDSTACK_H

#include "Global.h"

typedef struct structNode
{
	int mData;
	structNode * mNextNode;
} sNode;

//노드생성
sNode * fCreateNode(int _Data);

//노드삭제
void fDeleteNode(sNode * _Node);

typedef struct structLinkedStack
{
	sNode * mHead;
	sNode * mTop;
	int mSize;

}sLinkedStack;

//푸쉬 (새로운 데이터 입력)
void fPush(sLinkedStack * _Stack, int _Data);

//팝 (맨위의 데이터를 출력함과 동시에 삭제)
int fPop(sLinkedStack * _Stack);

//사이즈 (현재 스택에 들어있는 데이터의 량)
int fSize(sLinkedStack * _Stack);

//엠프티 (스택이 비어있나 확인)
int fEmpty(sLinkedStack * _Stack);

//탑 (맨위의 데이터)
int fTop(sLinkedStack * _Stack);

#endif LINKEDSTACK_H