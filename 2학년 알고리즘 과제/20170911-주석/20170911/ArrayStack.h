#ifndef ARRAYSTACK_H
#define ARRAYSTACK_H

#include <stdio.h>
#include <stdlib.h>

typedef int ElementType;

typedef struct tagNode			//노드구조체
{
    ElementType Data;			//데이터
} Node;

typedef struct tagArrayStack
{
    int   Capacity;				//총용량
    int   Top;					//가장 마지막에 넣은 데이터의 위치
    Node* Nodes;				//노드
} ArrayStack;

void        AS_CreateStack(ArrayStack** Stack, int Capacity);
void        AS_DestroyStack(ArrayStack* Stack);
void        AS_Push(ArrayStack* Stack, ElementType Data);
ElementType AS_Pop(ArrayStack* Stack);
ElementType AS_Top(ArrayStack* Stack);
int         AS_GetSize(ArrayStack* Stack);
int         AS_IsEmpty(ArrayStack* Stack);

#endif ARRAYSTACK_H