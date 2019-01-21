#include "Graph.h"

int main()
{
	sArrayGraph * graph = CreateArrayGraph(5);

	for (int i = 0; i < 5; i++)
	{
		AddVertex(graph, i + 1, i);
	}

	AddEdge(graph, 0, 1);
	AddEdge(graph, 0, 2);
	AddEdge(graph, 0, 3);
	AddEdge(graph, 0, 4);

	AddEdge(graph, 1, 0);
	AddEdge(graph, 1, 2);
	AddEdge(graph, 1, 4);

	AddEdge(graph, 2, 0);
	AddEdge(graph, 2, 1);

	AddEdge(graph, 3, 0);
	AddEdge(graph, 3, 4);

	AddEdge(graph, 4, 0);
	AddEdge(graph, 4, 1);
	AddEdge(graph, 4, 3);

	PrintArrayGraph(graph);
	DestroyArrayGraph(graph);
}