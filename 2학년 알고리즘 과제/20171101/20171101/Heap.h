#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEFT true
#define RIGHT false
#define BASECAPACITY 14
#define ROOT 0

struct sNode					//노드 구조체
{
	int data;					//해당 노드의 데이터
	int index;					//해당 노드의 인덱스
};

struct sHeap					//힙 구조체
{
	sNode * node;				//힙속의 노드 배열
	int capacity;				//총용량
	int usesize;				//사용되는 크기
};

void Swap(sHeap * _heap, int _indexA, int _indexB);							//힙에있는 배열에 인덱스를 사용해서 서로 값을 바꿈
void CreateHeap(sHeap * _heap);												//힙 생성함수
void Insert(sHeap * _heap, int _data);										//힙에 데이터 추가 함수
sNode PullRoot(sHeap * _heap);												//루트를 꺼내는 함수
void ChildCompare(sHeap * _heap, sNode _visit);								//자식과 비교 함수
void ParentsCompare(sHeap * _heap, sNode _visit);							//부모와 비교 함수
sNode GetChild(sHeap * _heap, int _nodeIndex, bool _isLeft);				//자식을 가져오는 함수 _isLeft가 true이면 왼쪽 자신 아니면 오른쪽 자식
sNode GetParents(sHeap * _heap, int _nodeIndex);							//부모를 가져오는 함수
void DestroyHeap(sHeap * _heap);											//힙 삭제 함수
