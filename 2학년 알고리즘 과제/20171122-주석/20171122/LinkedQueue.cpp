#include "LinkedQueue.h"

//ť ���� �Լ�
LinkedQueue* LQ_CreateQueue()
{
	LinkedQueue* Queue = (LinkedQueue*)malloc(sizeof(LinkedQueue));		//ť ����
	/*  ť�� ��������ҿ� ���� */

	Queue->Front = NULL;				//�ʱ�ȭ
	Queue->Rear = NULL;
	Queue->Count = 0;

	return Queue;
}

//ť ���� �Լ�
void LQ_DestroyQueue(LinkedQueue* Queue)
{
	//ť�� ���� �������� �ݺ�
	while (!LQ_IsEmpty(Queue))											
	{
		Node* Popped = LQ_Dequeue(&Queue);
		LQ_DestroyNode(Popped);
	}

	/*  ť�� ���� ����ҿ��� ���� */
	free(Queue);
}

//��� ���� �Լ�
Node* LQ_CreateNode(Vertex* V)
{
	//��� ����
	Node* NewNode = (Node*)malloc(sizeof(Node));
	NewNode->Data = V;

	NewNode->NextNode = NULL; /*  ���� ��忡 ���� �����ʹ� NULL�� �ʱ�ȭ �Ѵ�. */

	return NewNode;/*  ����� �ּҸ� ��ȯ�Ѵ�. */
}

//��� ���� �Լ�
void  LQ_DestroyNode(Node* _Node)
{
	free(_Node->Data);		//����� ������ ����
	free(_Node);			//��� ����
}

//ť�� ���ο� ��� �߰�
void LQ_Enqueue(LinkedQueue** Queue, Node* NewNode)
{
	if ((*Queue)->Front == NULL)			//���� �Ǿ��� ����ٸ�
	{
		(*Queue)->Front = NewNode;
		(*Queue)->Rear = NewNode;
		(*Queue)->Count++;					//�Ǿտ� ����
	}
	else
	{									
		(*Queue)->Rear->NextNode = NewNode;	//�ƴ϶�� �ǳ��� ��忡 ������ ������
		(*Queue)->Rear = NewNode;			//���ο� ��带 �ǳ����� ����
		(*Queue)->Count++;					
	}
}

//ť�� �Ǿ� ��带 ���� �Լ�
Node* LQ_Dequeue(LinkedQueue** Queue)
{
	Node* Front = (*Queue)->Front;				//���� �Ǿ��� ����

	if ((*Queue)->Front->NextNode == NULL)		//���� ��尡 �ϳ����ϰ��
	{
		(*Queue)->Front = NULL;
		(*Queue)->Rear = NULL;					//�ʱ�ȭ
	}
	else
	{
		(*Queue)->Front = (*Queue)->Front->NextNode;	//�ƴҰ�� 2��° ��带 �Ǿճ��� ����
	}

	(*Queue)->Count--;							//ī��Ʈ ����

	return Front;								//���� �Ǿ� ��带 �ܺο� ��ȯ
}

int LQ_IsEmpty(LinkedQueue* Queue)
{
	return (Queue->Front == NULL);
}