#include  "Queue.h"

int main()
{
	sQueue * queue = nullptr;		//ť ����ü
	queue = new sQueue;				//�����Ҵ�

	int inputSize;					//ť�� ũ�� �Է¿� ����
	char * tempCharArray;			//ť�� ������ �Է¿� ����
	int * tempIntArray;				//ť�� �켱���� �Է¿� ����

	printf("������ ���� : ");
	scanf("%d", &inputSize);				//ũ�� �Է�
	fflush(stdin);

	tempCharArray = new char[inputSize];		//�׸�ŭ �����Ҵ�
	tempIntArray = new int[inputSize];			//�׸�ŭ �����Ҵ�
	CreateQueue(queue, inputSize);				//ť�� ��� ������ ������ŭ ����

	printf("\n");

	for (int i = 0; i < inputSize; i++)
	{
		fflush(stdin);
		printf("������ : ");
		scanf("%c", &tempCharArray[i]);			//������ �ӽ� ��������� ����
	}

	printf("\n");

	for (int i = 0; i < inputSize; i++)
	{
		fflush(stdin);
		printf("�켱 ���� : ");
		scanf("%d", &tempIntArray[i]);			//�켱���� �ӽ� ��������� ����
	}

	printf("\n");

	for (int i = 0; i < inputSize; i++)
	{
		if (!EnQueue(queue, tempCharArray[i], tempIntArray[i]))		//��� ������ �Է�
		{
			break;
		}
	}

	delete tempCharArray;
	delete tempIntArray;					//�Ѵ� ��ȯ

	int count = 0;

	while (1)
	{
		sNode * getNode = DeQueue(queue);						//�Ǿ��� ��带 ����

		if (IsMinCheck(queue, getNode->priority))				//���� ������ üũ
		{
			printf("%c", getNode->data);						//�۴ٸ� ���
			count++;
		}

		else													//�ƴ϶��
		{
			EnQueue(queue, getNode->data, getNode->priority);	//�ٽ� ť�� �ǵڿ� �������
		}
			
		delete getNode;											//����� ��� ����

		if (count == queue->mSize)								//size��ŭ ����Ͽ��ٸ�
		{
			printf("\n\n");						
			break;												//�ݺ��� Ż��
		}
	}

	DestroyQueue(queue);										//���α׷� ������ ť ����
}