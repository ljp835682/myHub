#include "Heap.h"

int main()
{
	sHeap * heap;
	heap = (sHeap*)malloc(sizeof(sHeap));
	CreateHeap(heap);

	while (1)
	{
		bool exitFlag = false;

		printf("\n\n");
		for (int i = 0; i < heap->usesize; i++)
		{
			printf(" %d ", heap->node[i]);
		}
		printf("\n\n");

		int input;

		printf("1. insert, 2. rootpop, 3.exit : ");
		scanf("%d", &input);
		switch (input)
		{
		case 1:
			int data;

			printf("data : ");
			scanf("%d", &data);

			Insert(heap, data);
			break;

		case 2:
			printf("oldRoot : %d\n", PullRoot(heap));
			break;

		case 3:
			exitFlag = true;
			DestroyHeap(heap);
			break;
		}

		if (exitFlag)
		{
			return 0;
		}
	}
}