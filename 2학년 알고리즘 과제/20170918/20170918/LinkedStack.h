#ifndef LINKEDSTACK_H
#define LINKEDSTACK_H

#include "Global.h"

typedef struct structNode
{
	int mData;
	structNode * mNextNode;
} sNode;

//������
sNode * fCreateNode(int _Data);

//������
void fDeleteNode(sNode * _Node);

typedef struct structLinkedStack
{
	sNode * mHead;
	sNode * mTop;
	int mSize;

}sLinkedStack;

//Ǫ�� (���ο� ������ �Է�)
void fPush(sLinkedStack * _Stack, int _Data);

//�� (������ �����͸� ����԰� ���ÿ� ����)
int fPop(sLinkedStack * _Stack);

//������ (���� ���ÿ� ����ִ� �������� ��)
int fSize(sLinkedStack * _Stack);

//����Ƽ (������ ����ֳ� Ȯ��)
int fEmpty(sLinkedStack * _Stack);

//ž (������ ������)
int fTop(sLinkedStack * _Stack);

#endif LINKEDSTACK_H