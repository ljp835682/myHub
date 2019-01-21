#include <stdio.h>
#include <string.h>

struct sNode			//노드 구조체
{
	char data;
	int priority;
	sNode * next;
};

struct sQueue			//원형큐 구조체
{
	sNode * front;
	sNode * rear;
	int mSize;
	int mCount;
};

void CreateQueue(sQueue * _queue, int _size);				//큐 생성
bool EnQueue(sQueue * _queue, char _data, int _priority);	//큐에 새로운 데이터 집어넣기
sNode * DeQueue(sQueue * _queue);							//큐의 맨앞 데이터 빼내기
bool IsMinCheck(sQueue * _queue, int _priority);			//큐에 쌓인 노드들의 내부데이터와 _priority 를의 크기를 비교하는 함수
void DestroyQueue(sQueue * _queue);							//큐 삭제