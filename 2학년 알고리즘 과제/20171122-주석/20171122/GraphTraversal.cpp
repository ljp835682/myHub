#include "GraphTraversal.h"

//���� �켱 Ž��
void DFS(Vertex* V)
{
	Edge* E = NULL;						

	printf("%d ", V->Data);						//���� ������ �����͸� ���

	V->Visited = Visited;						//�湮 ���·� ����

	E = V->AdjacencyList;						//������ ���� ������ �������� ����

	//������ �ִµ��� �ݺ�
	while (E != NULL)
	{
		//���� ���� ������ Ÿ�������� �ְ� �ش� Ÿ�������� ���°� �湮���� ���� �����ϰ��
		if (E->Target != NULL && E->Target->Visited == NotVisited)
			DFS(E->Target);						//���ȣ��

		E = E->Next;							//������ ���� �������� ����
	}
}

//�ʺ� �켱 Ž��
void BFS(Vertex* V, LinkedQueue* Queue)
{
	Edge* E = NULL;

	printf("%d ", V->Data);						//���� ������ ������ ���
	V->Visited = Visited;						//���� ������ ���¸� �湮�� ���·� ����

	/*  ť�� ��� ����. */
	LQ_Enqueue(&Queue, LQ_CreateNode(V));		

	//ť�� ���� ���� ���� �ݺ�
	while (!LQ_IsEmpty(Queue))
	{
		Node* Popped = LQ_Dequeue(&Queue);		//�Ǿ��� ��带 ����
		V = Popped->Data;						//������ �˵� ����� ������
		E = V->AdjacencyList;					//������ �˵� ����� ������ ����

		//������ ���� ���������� �ݺ�
		while (E != NULL)
		{
			V = E->Target;						//������ ������ Ÿ������ ����

			//���� ������ ���� �ʾҰ� ������ ���°� �湮���� ���� �����ϰ��
			if (V != NULL && V->Visited == NotVisited)
			{
				printf("%d ", V->Data);			//���
				V->Visited = Visited;			//���� ����
				LQ_Enqueue(&Queue, LQ_CreateNode(V));		//���ο� ������ ť�� ����
			}

			E = E->Next;						//������ ���� �������� ����
		}
	}
}