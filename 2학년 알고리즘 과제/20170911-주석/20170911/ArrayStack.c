#include "ArrayStack.h"

void  AS_CreateStack(ArrayStack** Stack, int Capacity)
{
    /*  ������ ��������ҿ� ���� */
    (*Stack)           = (ArrayStack*)malloc(sizeof(ArrayStack));

    /*  �Էµ� Capacity��ŭ�� ��带 ��������ҿ� ���� */
    (*Stack)->Nodes    = (Node*)malloc(sizeof(Node)*Capacity);

    /*  Capacity �� Top �ʱ�ȭ */
    (*Stack)->Capacity = Capacity;
    (*Stack)->Top = 0;
}

void AS_DestroyStack(ArrayStack* Stack)
{
    /*  ��带 ���� ����ҿ��� ���� */
    free(Stack->Nodes);

    /*  ������ ���� ����ҿ��� ���� */
    free(Stack);
}

void AS_Push(ArrayStack* Stack, ElementType Data)		//�����͸� �ִ� �Լ�
{
    int Position = Stack->Top;					//��ġ�� ���� ž�� ���� ����

    Stack->Nodes[Position].Data = Data;			//�� ��ġ�� �����Ϳ� �������� ����
    Stack->Top++;								//�����͸� �־����Ƿ� ž���� ��ĭ�� ����Ŵ
}

ElementType AS_Pop(ArrayStack* Stack)			//�����͸� ���� �Լ�
{
    int Position = --( Stack->Top );			//���� ���� ���� ��ġ�� ����� ��ġ�� �˾Ƴ��� ���������� Top�� ����

    return Stack->Nodes[Position].Data;			//�� ����� �����͸� ����
}

ElementType AS_Top(ArrayStack* Stack)			//���� ž�� �˾Ƴ��� �Լ�
{
    int Position = Stack->Top - 1;				//���� ���� ���� ��ġ�� ����� ��ġ�� �˾Ƴ��� �ӽ������� Top�� ����

    return Stack->Nodes[Position].Data;			//�� ����� �����͸� ���� 
}

int AS_GetSize(ArrayStack* Stack)				//������ ũ�⸦ �˾Ƴ��� �Լ�
{
    return Stack->Top;							//������ ���� ���� ����� ��ġ�� ����
}

int AS_IsEmpty(ArrayStack* Stack)				//������ ���� �Լ�
{
    return (Stack->Top == 0);					//������ �������� ��Ÿ���� ������ 0���� ������ȯ
}