#include "LinkedStack.h"

//������
sNode * fCreateNode(int _X,int _Y)
{
	sNode * NewNode = (sNode*)malloc(sizeof(sNode));	
	NewNode->X = _X;
	NewNode->Y = _Y;
	NewNode->mNextNode = NULL;						
	return NewNode;											
}

//������
void fDeleteNode(sNode * _Node)
{
	free(_Node);							
}

//Ǫ�� (���ο� ������ �Է�)
void fPush(sLinkedStack * _Stack, int _X, int _Y)
{
	sNode * NewNode = fCreateNode(_X, _Y);
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
sNode fPop(sLinkedStack * _Stack)
{
	sNode * DeleteNode = _Stack->mTop;
	sNode PoppenNode;
	PoppenNode.X = _Stack->mTop->X;
	PoppenNode.Y = _Stack->mTop->Y;
	_Stack->mTop = _Stack->mTop->mNextNode;

	fDeleteNode(DeleteNode);
	_Stack->mSize--;

	return PoppenNode;
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
sNode * fTop(sLinkedStack * _Stack)
{
	if (_Stack->mSize == 0)
	{
		return NULL;
	}

	return (_Stack->mTop);
}
