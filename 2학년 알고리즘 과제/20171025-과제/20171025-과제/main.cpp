#include  "Queue.h"

int main()
{
	sQueue * queue = nullptr;		//큐 구조체
	queue = new sQueue;				//동적할당

	int inputSize;					//큐의 크기 입력용 공간
	char * tempCharArray;			//큐의 데이터 입력용 공간
	int * tempIntArray;				//큐의 우선순위 입력용 공간

	printf("데이터 갯수 : ");
	scanf("%d", &inputSize);				//크기 입력
	fflush(stdin);

	tempCharArray = new char[inputSize];		//그만큼 동적할당
	tempIntArray = new int[inputSize];			//그만큼 동적할당
	CreateQueue(queue, inputSize);				//큐의 노드 데이터 갯수만큼 생성

	printf("\n");

	for (int i = 0; i < inputSize; i++)
	{
		fflush(stdin);
		printf("데이터 : ");
		scanf("%c", &tempCharArray[i]);			//데이터 임시 저장공간에 저장
	}

	printf("\n");

	for (int i = 0; i < inputSize; i++)
	{
		fflush(stdin);
		printf("우선 순위 : ");
		scanf("%d", &tempIntArray[i]);			//우선순위 임시 저장공간에 저장
	}

	printf("\n");

	for (int i = 0; i < inputSize; i++)
	{
		if (!EnQueue(queue, tempCharArray[i], tempIntArray[i]))		//노드 데이터 입력
		{
			break;
		}
	}

	delete tempCharArray;
	delete tempIntArray;					//둘다 반환

	int count = 0;

	while (1)
	{
		sNode * getNode = DeQueue(queue);						//맨앞의 노드를 꺼냄

		if (IsMinCheck(queue, getNode->priority))				//가장 작은지 체크
		{
			printf("%c", getNode->data);						//작다면 출력
			count++;
		}

		else													//아니라면
		{
			EnQueue(queue, getNode->data, getNode->priority);	//다시 큐의 맨뒤에 집어넣음
		}
			
		delete getNode;											//사용한 노드 삭제

		if (count == queue->mSize)								//size만큼 출력하였다면
		{
			printf("\n\n");						
			break;												//반복문 탈출
		}
	}

	DestroyQueue(queue);										//프로그렘 종료전 큐 삭제
}