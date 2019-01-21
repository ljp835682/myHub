#include "Graph.h"

int main()
{
	int num;
	printf("정점의 개수 : ");
	scanf("%d", &num);

	sArrayGraph * graph = CreateArrayGraph(num);

	for (int i = 0; i < num; i++)
	{
		AddVertex(graph, i + 1, i);
	}

	while (1)
	{
		int a, b;
		printf("간선 연결(같은 수 입력시 종료) : ");

		scanf("%d %d", &a,&b);

		if (a == b)									//종료 조건
		{
			break;
		}

		else if (a >= num || b >= num)				//범위 초과시 재입력
		{
			printf("범위 초과 반영 되지 않음\n");
			continue;
		}

		else
		{
			AddEdge(graph, a, b);					//간선추가
		}
	}

	PrintArrayGraph(graph);
	DestroyArrayGraph(graph);
}