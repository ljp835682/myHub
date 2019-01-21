#include "Heap.h"

char str[4][10] = { "����", "����", "����", "����" };			//�����

int main()
{
	sHeap * heap;
	heap = (sHeap*)malloc(sizeof(sHeap));
	CreateHeap(heap);

	while (1)
	{
		bool exitFlag = false;
		int scoreSum = 0;

		printf("���� ���ο� �� : %d\n", heap->usesize);		//���ο��� ���

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
				scoreSum += score;							//���� ���ϱ�
			}

			printf("Number : ");								
			scanf("%d", &number);

			Insert(heap, scoreSum, number);					//���� ������ ��ȣ�� �־ �Է�
			break;

		case 2:
			int max = heap->usesize;						//ó�� ������ ���ũ�⸦ ����	
			for (int i = 0; i < max; i++)					//��� ��Ʈ�� �����⿡ usesize�� ����ϸ� �ݺ����� �߰��� ����
			{
				sNode oldRoot = PullRoot(heap);
				printf("%d�� : %d , ���� : %d\n", i + 1, oldRoot.number, oldRoot.data);
			}	

			exitFlag = true;
			DestroyHeap(heap);								//�� ����
			break;
		}

		if (exitFlag)
		{
			system("pause");
			return 0;									//����
		}
	}
}