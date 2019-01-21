#include "Heap.h"

//스왑
void Swap(sHeap * _heap, int _indexA, int _indexB)
{
	int temp = _heap->node[_indexA].data;						
	_heap->node[_indexA].data = _heap->node[_indexB].data;
	_heap->node[_indexB].data = temp;									//데이터를 바꿈
}

//힙 생성 함수
void CreateHeap(sHeap * _heap)
{
	_heap->capacity = BASECAPACITY;										
	_heap->usesize = 0;
	_heap->node = (sNode*)malloc(_heap->capacity * sizeof(sNode));

	for (int i = 0; i < _heap->capacity; i++)
	{
		_heap->node[i].index = -1;										//초기화
	}
}

//데이터 추가 함수
void Insert(sHeap * _heap, int _data)
{
	if (_heap->capacity == _heap->usesize)								//사용용량한계도달시
	{
		_heap->capacity *= 2;
		_heap->node = (sNode*)realloc(_heap->node, _heap->capacity * sizeof(sNode));		
																		//더욱 공간 확보

		for (int i = _heap->usesize; i < _heap->capacity; i++)
		{
			_heap->node[i].index = -1;									//새로만든곳 초기화
		}
	}

	_heap->node[_heap->usesize].data = _data;
	_heap->node[_heap->usesize].index = _heap->usesize;					//마지막 공간에 추가

	ParentsCompare(_heap, _heap->node[_heap->usesize]);					//부모와 비교

	_heap->usesize++;													//추가 끝나면 사용용량 증가
}

//루트 꺼내기 함수
sNode PullRoot(sHeap * _heap)									
{
	if (_heap->usesize == 0)											//만약 사용용량이 0이라면
	{
		sNode error;
		error.data = NULL;
		error.index = -1;

		return error;													//위의 값을 리턴
	}

	sNode oldRoot;
	memcpy(&oldRoot, &(_heap->node[ROOT]), sizeof(sNode));				//현재 루트를 복사

	_heap->node[ROOT].data = _heap->node[_heap->usesize - 1].data;		//루트에 맨끝의 데이터를 대입
	_heap->usesize--;													//사용량 감소

	ChildCompare(_heap,_heap->node[ROOT]);								//자식과 비교

	return oldRoot;														//저장된 이전 루트를 외부에 리턴
}

//자식과 비교 함수
void ChildCompare(sHeap * _heap, sNode _visit)							
{
	sNode smallerChild = (GetChild(_heap, _visit.index, LEFT).data		//두개의 자식 노드중 작은쪽의 노드를 저장
				> GetChild(_heap, _visit.index, RIGHT).data)
				? GetChild(_heap, _visit.index, RIGHT)
				: GetChild(_heap, _visit.index, LEFT);

	if (smallerChild.index == -1)										//만약 찾은 자식노드의 인덱스가 -1이면
	{
		return;															//함수종료
	}

	if (_visit.data > smallerChild.data)								//현재 노드의 데이터가 부모노드의 데이터보다 클경우
	{
		Swap(_heap, _visit.index, smallerChild.index);					//서로 바꿈
		_visit.index = smallerChild.index;								//함수 내에서도 바꿈
		ChildCompare(_heap, _visit);									//끝까지 계속 비교
	}
}

//부모와 비교
void ParentsCompare(sHeap * _heap, sNode _visit)
{
	if (_visit.index == ROOT)											//현재 노드가 루트면
	{
		return;															//종료
	}

	sNode Parents = GetParents(_heap, _visit.index);					//부모 저장

	if (_visit.data < Parents.data)										//만약 현재 노드의 데이터가 부모 노드의 데이터보다 작다면
	{
		Swap(_heap, _visit.index, Parents.index);						//서로 바꿈
		_visit.index = Parents.index;									//함수 내에서도 바꿈
		ParentsCompare(_heap, _visit);									//끝까지 계속 비교
	}
}

//자식 노드 찾기 함수
sNode GetChild(sHeap * _heap, int _nodeIndex, bool _isLeft)
{
	if (_isLeft)
	{
		if (((_nodeIndex)* 2) + 1 > _heap->usesize)
		{
			sNode error;
			error.data = 2147483647;
			error.index = -1;

			return error;										//범위 벗어나면 에러
		}
		
		return _heap->node[((_nodeIndex)* 2) + 1];				//아니면 왼쪽 자식 노드 리턴
	}

	else
	{
		if (((_nodeIndex)* 2) + 2 > _heap->usesize)				
		{
			sNode error;
			error.data = 2147483646;
			error.index = -1;

			return error;										//범위 벗어나면 에러
		}

		return _heap->node[((_nodeIndex)* 2) + 2];				//아니면 오른쪽 자식 노드 리턴
	}
}

//부모 노드 찾기 함수
sNode GetParents(sHeap * _heap, int _nodeIndex)
{
	return _heap->node[int((_nodeIndex - 1) / 2)];				//부모 노드 리턴
}

//힙 삭제 함수
void DestroyHeap(sHeap * _heap)
{
	free(_heap->node);											//노드 배열 삭제
	free(_heap);												//힙 삭제
}