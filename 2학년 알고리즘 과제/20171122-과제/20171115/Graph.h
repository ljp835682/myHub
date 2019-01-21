#include <stdio.h>

//���� ����ü
struct Vertex		
{
	int data;				//������
	bool * edge;			//����
	int index;				//�ڽ��� �ε���
};

//�׷���
struct sArrayGraph
{
	Vertex * vertex;		//�����迭
	int size;				//ũ��
};

sArrayGraph * CreateArrayGraph(int _size);		
void DestroyArrayGraph(sArrayGraph * _graph);
void AddVertex(sArrayGraph * _graph, int _data, int _index);
void AddEdge(sArrayGraph * _graph, int _mainIndex, int _targetIndex);
void PrintArrayGraph(sArrayGraph * _graph);