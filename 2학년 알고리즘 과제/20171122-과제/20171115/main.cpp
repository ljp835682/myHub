#include "Graph.h"

int main()
{
	int num;
	printf("������ ���� : ");
	scanf("%d", &num);

	sArrayGraph * graph = CreateArrayGraph(num);

	for (int i = 0; i < num; i++)
	{
		AddVertex(graph, i + 1, i);
	}

	while (1)
	{
		int a, b;
		printf("���� ����(���� �� �Է½� ����) : ");

		scanf("%d %d", &a,&b);

		if (a == b)									//���� ����
		{
			break;
		}

		else if (a >= num || b >= num)				//���� �ʰ��� ���Է�
		{
			printf("���� �ʰ� �ݿ� ���� ����\n");
			continue;
		}

		else
		{
			AddEdge(graph, a, b);					//�����߰�
		}
	}

	PrintArrayGraph(graph);
	DestroyArrayGraph(graph);
}