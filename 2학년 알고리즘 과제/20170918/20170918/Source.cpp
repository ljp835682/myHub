#include "LinkedStack.h"

int main()
{
	sLinkedStack * Stack = (sLinkedStack *)malloc(sizeof(sLinkedStack));
	Stack->mHead = NULL;
	Stack->mTop = NULL;
	Stack->mSize = 0;


	int OderCount;
	printf("명령의 수 : ");
	scanf("%d", &OderCount);

	for (int i = 0; i < OderCount; i++)
	{
		int choice;
		printf("1.push 2.pop 3.size 4.empty 5.top\n");
		scanf("%d", &choice);

		switch (choice)
		{
		case 1:
			int num;
			printf("입력 : ");
			scanf("%d", &num);
			fPush(Stack, num);
			break;
		case 2:
			printf("팝된 데이터 : %d\n",fPop(Stack));
			break;
		case 3:
			printf("데이터의 갯수 : %d\n", fSize(Stack));
			break;
		case 4:
			printf("스택의 상황 : %d\n", fEmpty(Stack));
			break;
		case 5:
			printf("현재의 탑 : %d\n", fTop(Stack));
			break;
		}
	}

	for (int i = 0; i < Stack->mSize; i++)
	{
		fPop(Stack);
	}

	return 0;
}