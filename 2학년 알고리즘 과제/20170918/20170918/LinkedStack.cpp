#include "LinkedStack.h"

//������
sNode * fCreateNode(int _Data)
{
	sNode * NewNode = (sNode*)malloc(sizeof(sNode));	
	NewNode->mData = _Data;							
	NewNode->mNextNode = NULL;						
	return NewNode;											
}

//������
void fDeleteNode(sNode * _Node)
{
	free(_Node);							
}

//Ǫ�� (���ο� ������ �Է�)
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

//�� (������ �����͸� ����԰� ���ÿ� ����)
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

//������ (���� ���ÿ� ����ִ� �������� ��)
int fSize(sLinkedStack * _Stack)
{
	return _Stack->mSize;
}

//����Ƽ (������ ����ֳ� Ȯ��)
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

//ž (������ ������)
int fTop(sLinkedStack * _Stack)
{
	if (_Stack->mSize == 0)
	{
		return ERROR;
	}

	return (_Stack->mTop->mData);
}
