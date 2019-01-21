#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEFT true
#define RIGHT false
#define BASECAPACITY 14
#define ROOT 0

struct sNode					//��� ����ü
{
	int data;					//�ش� ����� ������
	int index;					//�ش� ����� �ε���
};

struct sHeap					//�� ����ü
{
	sNode * node;				//������ ��� �迭
	int capacity;				//�ѿ뷮
	int usesize;				//���Ǵ� ũ��
};

void Swap(sHeap * _heap, int _indexA, int _indexB);							//�����ִ� �迭�� �ε����� ����ؼ� ���� ���� �ٲ�
void CreateHeap(sHeap * _heap);												//�� �����Լ�
void Insert(sHeap * _heap, int _data);										//���� ������ �߰� �Լ�
sNode PullRoot(sHeap * _heap);												//��Ʈ�� ������ �Լ�
void ChildCompare(sHeap * _heap, sNode _visit);								//�ڽİ� �� �Լ�
void ParentsCompare(sHeap * _heap, sNode _visit);							//�θ�� �� �Լ�
sNode GetChild(sHeap * _heap, int _nodeIndex, bool _isLeft);				//�ڽ��� �������� �Լ� _isLeft�� true�̸� ���� �ڽ� �ƴϸ� ������ �ڽ�
sNode GetParents(sHeap * _heap, int _nodeIndex);							//�θ� �������� �Լ�
void DestroyHeap(sHeap * _heap);											//�� ���� �Լ�
