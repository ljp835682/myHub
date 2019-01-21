#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdio.h>
#include <stdlib.h>

typedef int ElementType;

typedef struct tagNode
{
    int Number;
	int Score;

    struct tagNode* NextNode;
} Node;

/* �Լ� ���� ���� */
Node* SLL_CreateNode(ElementType _Number, ElementType _Score);	//��� ���� �Լ�
void  SLL_DestroyNode(Node* Node);								//��� �Ҹ� �Լ�
void  SLL_AppendNode(Node** Head, Node* NewNode);				//��� �߰� �Լ�
void  SLL_InsertAfter(Node* Current, Node* NewNode);			//��� ���� �Լ�
void  SLL_InsertNewHead(Node** Head, Node* NewHead);			//���ο� ��带 ����� �Լ�
void  SLL_RemoveNode(Node** Head, Node* Remove);				//��� ���� �Լ�
Node* SLL_GetNodeAt(Node* Head, int Location);					//��� Ž�� �Լ�
Node* SLL_SearchNode(Node* Head, int _Number);					//��� �˻� �Լ�
int   SLL_GetNodeCount(Node* Head);								//��� ��ü�� ī��Ʈ �Լ�

#endif
