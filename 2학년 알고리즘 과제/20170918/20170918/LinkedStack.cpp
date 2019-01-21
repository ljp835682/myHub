#include "LinkedStack.h"

//노드생성
sNode * fCreateNode(int _Data)
{
	sNode * NewNode = (sNode*)malloc(sizeof(sNode));	
	NewNode->mData = _Data;							
	NewNode->mNextNode = NULL;						
	return NewNode;											
}

//노드삭제
void fDeleteNode(sNode * _Node)
{
	free(_Node);							
}

//푸쉬 (새로운 데이터 입력)
void fPush(sLinkedStack * _Stack, int _Data)
{
	sNode * NewNode = fCreateNode(_Data);				
	if (_Stack->mSize == 0)								
	{
		_Stack->mHead = NewNode;							
		_Stack->mTop = NewNode;							
	}

	else
	{
		NewNode->mNextNode = _Stack->mTop;					
		_Stack->mTop = NewNode;							
	}

	_Stack->mSize++;
}

//팝 (맨위의 데이터를 출력함과 동시에 삭제)
int fPop(sLinkedStack * _Stack)
{
	if (_Stack->mSize == 0)
	{
		return ERROR;
	}

	int Data;
	sNode * PoppenNode = _Stack->mTop;
	_Stack->mTop = _Stack->mTop->mNextNode;
	Data = PoppenNode->mData;

	fDeleteNode(PoppenNode);
	_Stack->mSize--;

	return Data;
}

//사이즈 (현재 스택에 들어있는 데이터의 량)
int fSize(sLinkedStack * _Stack)
{
	return _Stack->mSize;
}

//엠프티 (스택이 비어있나 확인)
int fEmpty(sLinkedStack * _Stack)
{
	if (_Stack->mSize == 0)
	{
		return EMPTY;
	}

	else
	{
		return NOTEMPTY;
	}
}

//탑 (맨위의 데이터)
int fTop(sLinkedStack * _Stack)
{
	if (_Stack->mSize == 0)
	{
		return ERROR;
	}

	return (_Stack->mTop->mData);
}
