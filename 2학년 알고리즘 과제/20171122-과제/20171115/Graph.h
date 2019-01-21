#include <stdio.h>

//정점 구조체
struct Vertex		
{
	int data;				//데이터
	bool * edge;			//간선
	int index;				//자신의 인덱스
};

//그래프
struct sArrayGraph
{
	Vertex * vertex;		//정점배열
	int size;				//크기
};

sArrayGraph * CreateArrayGraph(int _size);		
void DestroyArrayGraph(sArrayGraph * _graph);
void AddVertex(sArrayGraph * _graph, int _data, int _index);
void AddEdge(sArrayGraph * _graph, int _mainIndex, int _targetIndex);
void PrintArrayGraph(sArrayGraph * _graph);