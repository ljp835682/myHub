#include "Graph.h"

//�׷��� ����
sArrayGraph * CreateArrayGraph(int _size)
{
	sArrayGraph * graph = new sArrayGraph;					//�� �׷��� ����
	graph->vertex = new Vertex [_size];
	for (int i = 0; i < _size; i++)
	{
		graph->vertex[i].edge = new bool[_size];			//������ ������ŭ bool�� �迭 ����

		for (int j = 0; j < _size; j++)
		{
			graph->vertex[i].edge[j] = false;				//false �⺻��
		}

		graph->vertex[i].index = i;							
	}

	graph->size = _size;									//�ʱ�ȭ

	return graph;											//�ܺο� ��ȯ
}

//�׷��� ����
void DestroyArrayGraph(sArrayGraph * _graph)
{
	for (int i = 0; i < _graph->size; i++)					//������ ������ŭ �ݺ�
	{
		delete _graph->vertex[i].edge;						//���� ����
	}

	delete _graph->vertex;									//���� ����
}

//���� �߰�
void AddVertex(sArrayGraph * _graph, int _data, int _index)
{
	_graph->vertex[_index].data = _data;					//���� �߰�
}

//���� �߰�
void AddEdge(sArrayGraph * _graph, int _mainIndex,int _targetIndex)
{
	_graph->vertex[_mainIndex].edge[_targetIndex] = true;	
	_graph->vertex[_targetIndex].edge[_mainIndex] = true;	//���� �̾���
}

//�׷��� ���
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
			if (_graph->vertex[i].edge[j])				//����Ǿ������� ���
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