#include "Heap.h"

char str[4][10] = { "국어", "수학", "영어", "과학" };			//과목명

int main()
{
	sHeap * heap;
	heap = (sHeap*)malloc(sizeof(sHeap));
	CreateHeap(heap);

	while (1)
	{
		bool exitFlag = false;
		int scoreSum = 0;

		printf("현재 반인원 수 : %d\n", heap->usesize);		//반인원수 출력

		printf("\n\n");

		int input;

		printf("1. insert, 2.allprint : ");					
		scanf("%d", &input);
		switch (input)
		{
		case 1:
			int score, number;

			for (int i = 0; i < 4; i++)
			{
				printf("%s : ", str[i]);
				scanf("%d", &score);
				scoreSum += score;							//총점 구하기
			}

			printf("Number : ");								
			scanf("%d", &number);

			Insert(heap, scoreSum, number);					//힙에 총점과 번호를 넣어서 입력
			break;

		case 2:
			int max = heap->usesize;						//처음 상태의 사용크기를 저장	
			for (int i = 0; i < max; i++)					//계속 루트를 꺼내기에 usesize를 사용하면 반복문이 중간에 끝남
			{
				sNode oldRoot = PullRoot(heap);
				printf("%d등 : %d , 점수 : %d\n", i + 1, oldRoot.number, oldRoot.data);
			}	

			exitFlag = true;
			DestroyHeap(heap);								//힙 삭제
			break;
		}

		if (exitFlag)
		{
			system("pause");
			return 0;									//종료
		}
	}
}