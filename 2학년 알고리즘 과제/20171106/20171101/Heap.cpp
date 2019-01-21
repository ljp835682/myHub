#include "Heap.h"

//����
void Swap(sHeap * _heap, int _indexA, int _indexB)
{
	int tempScore, tempNumber;

	tempScore = _heap->node[_indexA].data;
	tempNumber = _heap->node[_indexA].number;

	_heap->node[_indexA].data = _heap->node[_indexB].data;
	_heap->node[_indexA].number = _heap->node[_indexB].number;

	_heap->node[_indexB].data = tempScore;
	_heap->node[_indexB].number = tempNumber;//�����͸� �ٲ�
}

//�� ���� �Լ�
void CreateHeap(sHeap * _heap)
{
	_heap->capacity = BASECAPACITY;										
	_heap->usesize = 0;
	_heap->node = (sNode*)malloc(_heap->capacity * sizeof(sNode));

	for (int i = 0; i < _heap->capacity; i++)
	{
		_heap->node[i].index = -1;										//�ʱ�ȭ
	}
}

//������ �߰� �Լ�
void Insert(sHeap * _heap, int _score, int _number)
{
	if (_heap->capacity == _heap->usesize)								//���뷮�Ѱ赵�޽�
	{
		_heap->capacity *= 2;
		_heap->node = (sNode*)realloc(_heap->node, _heap->capacity * sizeof(sNode));		
																		//���� ���� Ȯ��

		for (int i = _heap->usesize; i < _heap->capacity; i++)
		{
			_heap->node[i].index = -1;									//���θ���� �ʱ�ȭ
		}
	}

	_heap->node[_heap->usesize].data = _score;
	_heap->node[_heap->usesize].number = _number;
	_heap->node[_heap->usesize].index = _heap->usesize;					//������ ������ �߰�

	ParentsCompare(_heap, _heap->node[_heap->usesize]);					//�θ�� ��

	_heap->usesize++;													//�߰� ������ ���뷮 ����
}

//��Ʈ ������ �Լ�
sNode PullRoot(sHeap * _heap)									
{
	if (_heap->usesize == 0)											//���� ���뷮�� 0�̶��
	{
		sNode error;
		error.data = NULL;
		error.index = -1;

		return error;													//���� ���� ����
	}

	sNode oldRoot;
	memcpy(&oldRoot, &(_heap->node[ROOT]), sizeof(sNode));				//���� ��Ʈ�� ����

	_heap->node[ROOT].data = _heap->node[_heap->usesize - 1].data;		
	_heap->node[ROOT].number = _heap->node[_heap->usesize - 1].number;	//��Ʈ�� �ǳ��� �����͸� ����
	_heap->usesize--;													//��뷮 ����

	ChildCompare(_heap,_heap->node[ROOT]);								//�ڽİ� ��

	return oldRoot;														//����� ���� ��Ʈ�� �ܺο� ����
}

//�ڽİ� �� �Լ�
void ChildCompare(sHeap * _heap, sNode _visit)							
{
	sNode biggerChild = (GetChild(_heap, _visit.index, LEFT).data		//�ΰ��� �ڽ� ����� ū���� ��带 ����
				> GetChild(_heap, _visit.index, RIGHT).data)
				? GetChild(_heap, _visit.index, LEFT)
				: GetChild(_heap, _visit.index, RIGHT);

	if (biggerChild.index == -1)										//���� ã�� �ڽĳ���� �ε����� -1�̸�
	{
		return;															//�Լ�����
	}

	if (_visit.data < biggerChild.data)									//���� �湮�� ����� �����ͺ��� ū���� �ڽĳ�尡 �� ũ�ٸ�
	{
		Swap(_heap, _visit.index, biggerChild.index);					//���� �ٲ�
		_visit.index = biggerChild.index;								//�Լ� �������� �ٲ�
		ChildCompare(_heap, _visit);									//������ ��� ��
	}
}

//�θ�� ��
void ParentsCompare(sHeap * _heap, sNode _visit)
{
	if (_visit.index == ROOT)											//���� ��尡 ��Ʈ��
	{
		return;															//����
	}

	sNode Parents = GetParents(_heap, _visit.index);					//�θ� ����

	if (_visit.data > Parents.data)										//���� �湮 ��尡 �θ��庸�� ũ�ٸ�
	{
		Swap(_heap, _visit.index, Parents.index);						//���� �ٲ�
		_visit.index = Parents.index;									//�Լ� �������� �ٲ�
		ParentsCompare(_heap, _visit);									//������ ��� ��
	}
}

//�ڽ� ��� ã�� �Լ�
sNode GetChild(sHeap * _heap, int _nodeIndex, bool _isLeft)
{
	sNode error;
	error.data = 2147483647;
	error.index = -1;

	if (_isLeft)
	{
		if (((_nodeIndex)* 2) + 1 > _heap->usesize)
		{
			return error;										//���� ����� ����
		}
		
		return _heap->node[((_nodeIndex)* 2) + 1];				//�ƴϸ� ���� �ڽ� ��� ����
	}

	else
	{
		if (((_nodeIndex)* 2) + 2 > _heap->usesize)				
		{
			return error;										//���� ����� ����
		}

		return _heap->node[((_nodeIndex)* 2) + 2];				//�ƴϸ� ������ �ڽ� ��� ����
	}
}

//�θ� ��� ã�� �Լ�
sNode GetParents(sHeap * _heap, int _nodeIndex)
{
	return _heap->node[int((_nodeIndex - 1) / 2)];				//�θ� ��� ����
}

//�� ���� �Լ�
void DestroyHeap(sHeap * _heap)
{
	free(_heap->node);											//��� �迭 ����
	free(_heap);												//�� ����
}