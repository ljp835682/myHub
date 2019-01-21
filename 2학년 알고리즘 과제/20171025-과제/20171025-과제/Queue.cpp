#include "Queue.h"

void CreateQueue(sQueue * _queue, int _size)
{
	_queue->front = nullptr;
	_queue->rear = nullptr;
	_queue->mSize = _size;
	_queue->mCount = 0;

	for (int i = 0; i < _size + 1; i++)					//두번쨰 인자값 + 1 만큼 반복
	{
		sNode * newNode = new sNode;					//새로운 노드 생성
		newNode->data = NULL;
		newNode->priority = NULL;
		newNode->next = nullptr;

		if (i == 0)										//첫번째 상황일 경우
		{
			_queue->front = newNode;					//해당 노드를 front로 지정
		}

		else if (i == 1)								//두번째 상황일 경우
		{
			_queue->front->next = newNode;				//해당 노드를 front의 다음에 배치
			_queue->rear = newNode;						//해당 노드를 rear로 지정
		}

		else											//그외의 상황일 경우
		{
			_queue->rear->next = newNode;				//해당 노드를 rear 다음에 배치
			_queue->rear = newNode;						//해당 노드를 rear로 지정
		}
	}

	_queue->rear->next = _queue->front;					//반복문이 끝나 지정된 횟수만큼 노드들이 생성되고난후 맨 마지막 노드의 다음을 front로 지정
	_queue->rear = _queue->front->next;					//그후 새로운 데이터가 들어올곳을 더미노드인 front의 다음 노드로 지정
}

bool EnQueue(sQueue * _queue, char _data, int _priority)
{
	if (_queue->rear == _queue->front)					//만약 rear와 front가 같다면
	{
		return false;									//현재 원형큐가 포화상태인것이므로 넣을수 없게합니다
	}

	_queue->rear->data = _data;							
	_queue->rear->priority = _priority;					//rear의 데이터에 저장
	_queue->rear = _queue->rear->next;					//다음 입력은 다음 노드에 입력할수있도록 rear 다음 노드로 이동함

	_queue->mCount++;									//현재 입력된 노드수 증가

	return true;
}

sNode * DeQueue(sQueue * _queue)
{
	sNode * tempNode = new sNode;						//새로운 노드 생성
	_queue->front = _queue->front->next;				//front는 가장 앞의 노드로 이동

	memcpy(tempNode, _queue->front, sizeof(sNode));		//front의 데이터를 새로운 노드에 깊은 복사
	
	_queue->front->data = NULL;							
	_queue->front->priority = NULL;						//front의 데이터 삭제

	_queue->mCount--;									//현재 입력된 노드수 감소

	return tempNode;									//front의 이전값을 깊은복사한 노드를 외부로 리턴
}

bool IsMinCheck(sQueue * _queue, int _priority)
{
	sNode * searchNode = _queue->front->next;					//가장 앞의 노드에서부터 비교하기위해 front의 다음 노드로 검색 노드 지정

	while (1)
	{
		if (searchNode == _queue->front)						//만약 검색노드가 front라면 한바퀴를 다돌동안 아래에서 전부 만족한것이므로
		{
			return true;										//true 리턴
		}

		else if (searchNode->priority > _priority || searchNode->priority == NULL)		//만약 빈 노드이거나 검색 노드가 가리키는 노드의 우선도가 입력된 우선도보다 크다면
		{
			searchNode = searchNode->next;												//다음 노드를 비교할 준비
		}

		else
		{
			return false;																//작다면 false 리턴
		}
	}
}

void DestroyQueue(sQueue * _queue)	
{
	for (int i = 0; i < _queue->mSize; i++)						//큐의 사이즈만큼 반복
	{
		sNode * destroyNode = _queue->front;					//삭제용 노드에 front 지정
		_queue->front = _queue->front->next;					//front는 다음 노드로 이동

		delete destroyNode;										//삭제용 노드 반환
	}
}