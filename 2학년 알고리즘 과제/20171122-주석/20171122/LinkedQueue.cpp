#include "LinkedQueue.h"

//큐 생성 함수
LinkedQueue* LQ_CreateQueue()
{
	LinkedQueue* Queue = (LinkedQueue*)malloc(sizeof(LinkedQueue));		//큐 생성
	/*  큐를 자유저장소에 생성 */

	Queue->Front = NULL;				//초기화
	Queue->Rear = NULL;
	Queue->Count = 0;

	return Queue;
}

//큐 삭제 함수
void LQ_DestroyQueue(LinkedQueue* Queue)
{
	//큐가 비지 않을동안 반복
	while (!LQ_IsEmpty(Queue))											
	{
		Node* Popped = LQ_Dequeue(&Queue);
		LQ_DestroyNode(Popped);
	}

	/*  큐를 자유 저장소에서 해제 */
	free(Queue);
}

//노드 생성 함수
Node* LQ_CreateNode(Vertex* V)
{
	//노드 생성
	Node* NewNode = (Node*)malloc(sizeof(Node));
	NewNode->Data = V;

	NewNode->NextNode = NULL; /*  다음 노드에 대한 포인터는 NULL로 초기화 한다. */

	return NewNode;/*  노드의 주소를 반환한다. */
}

//노드 삭제 함수
void  LQ_DestroyNode(Node* _Node)
{
	free(_Node->Data);		//노드의 데이터 삭제
	free(_Node);			//노드 삭제
}

//큐에 새로운 노드 추가
void LQ_Enqueue(LinkedQueue** Queue, Node* NewNode)
{
	if ((*Queue)->Front == NULL)			//만약 맨앞이 비었다면
	{
		(*Queue)->Front = NewNode;
		(*Queue)->Rear = NewNode;
		(*Queue)->Count++;					//맨앞에 대입
	}
	else
	{									
		(*Queue)->Rear->NextNode = NewNode;	//아니라면 맨끝의 노드에 다음에 대입후
		(*Queue)->Rear = NewNode;			//새로운 노드를 맨끝노드로 지정
		(*Queue)->Count++;					
	}
}

//큐에 맨앞 노드를 빼는 함수
Node* LQ_Dequeue(LinkedQueue** Queue)
{
	Node* Front = (*Queue)->Front;				//현재 맨앞을 저장

	if ((*Queue)->Front->NextNode == NULL)		//만약 노드가 하나뿐일경우
	{
		(*Queue)->Front = NULL;
		(*Queue)->Rear = NULL;					//초기화
	}
	else
	{
		(*Queue)->Front = (*Queue)->Front->NextNode;	//아닐경우 2번째 노드를 맨앞노드로 지정
	}

	(*Queue)->Count--;							//카운트 감소

	return Front;								//이전 맨앞 노드를 외부에 반환
}

int LQ_IsEmpty(LinkedQueue* Queue)
{
	return (Queue->Front == NULL);
}