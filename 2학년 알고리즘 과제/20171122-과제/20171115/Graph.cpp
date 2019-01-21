#include "Graph.h"

//그래프 생성
sArrayGraph * CreateArrayGraph(int _size)
{
	sArrayGraph * graph = new sArrayGraph;					//새 그래프 생성
	graph->vertex = new Vertex [_size];
	for (int i = 0; i < _size; i++)
	{
		graph->vertex[i].edge = new bool[_size];			//정점의 개수만큼 bool형 배열 생성

		for (int j = 0; j < _size; j++)
		{
			graph->vertex[i].edge[j] = false;				//false 기본형
		}

		graph->vertex[i].index = i;							
	}

	graph->size = _size;									//초기화

	return graph;											//외부에 반환
}

//그래프 삭제
void DestroyArrayGraph(sArrayGraph * _graph)
{
	for (int i = 0; i < _graph->size; i++)					//정점의 개수만큼 반복
	{
		delete _graph->vertex[i].edge;						//간선 삭제
	}

	delete _graph->vertex;									//정점 삭제
}

//정점 추가
void AddVertex(sArrayGraph * _graph, int _data, int _index)
{
	_graph->vertex[_index].data = _data;					//정점 추가
}

//간선 추가
void AddEdge(sArrayGraph * _graph, int _mainIndex,int _targetIndex)
{
	_graph->vertex[_mainIndex].edge[_targetIndex] = true;	
	_graph->vertex[_targetIndex].edge[_mainIndex] = true;	//서로 이어줌
}

//그래프 출력
void PrintArrayGraph(sArrayGraph * _graph)
{
	printf("	");
	for (int i = 0; i < _graph->size; i++)
	{
		printf("%d	", i);
	}
	
	printf("\n");

	for (int i = 0; i < _graph->size; i++)
	{
		printf("%d	", i, _graph->vertex[i].data);

		for (int j = 0; j < _graph->size; j++)
		{
			if (_graph->vertex[i].edge[j])				//연결되었을때만 출력
			{
				printf("1	");
			}

			else
			{
				printf("0	");
			}
		}

		printf("\n\n");
	}
}