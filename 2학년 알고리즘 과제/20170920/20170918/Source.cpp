#include "LinkedStack.h"

//�̷�
int Maze[6][6] = {	{ 2, 1, 1, 1, 1, 0 },
					{ 0, 0, 0, 0, 0, 0 },
					{ 1, 1, 0, 1, 1, 1 },
					{ 1, 1, 0, 0, 0, 1 },
					{ 1, 1, 0, 1, 0, 1 },
					{ 0, 0, 0, 1, 0, 0 }, };

//������ ��ġ
typedef struct structUserLocation
{
	int X;
	int Y;
}sUserLocation;

//�����ִ� ��ġ�� �����ϴ� �Լ�
void SaveMovePoint(sUserLocation * _loc, sLinkedStack * _stack)
{
	//(�켱��) ����
	//(4) ������
	if (_loc->X + 1 < 6)
	{
		if (Maze[_loc->Y][_loc->X + 1] == 0)
		{

			fPush(_stack, _loc->X + 1, _loc->Y);
		}
	}

	//(3) ����
	if (_loc->X - 1 >= 0)
	{
		if (Maze[_loc->Y][_loc->X - 1] == 0)
		{
			fPush(_stack, _loc->X - 1, _loc->Y);
		}
	}

	//(2) �Ʒ���
	if (_loc->Y + 1 < 6)
	{
		if (Maze[_loc->Y + 1][_loc->X] == 0)
		{
			fPush(_stack, _loc->X, _loc->Y + 1);
		}
	}

	//(1) ����
	if (_loc->Y - 1 < 0)
	{
		if (Maze[_loc->Y - 1][_loc->X] == 0)
		{
			fPush(_stack, _loc->X, _loc->Y - 1);
		}
	}
}

//������ �̵��Լ�
void MovingPosition(sUserLocation * _loc, sLinkedStack * _stack)
{	
	//���ؼ� ������ ��带 ���� �� ��忡 ����� ��ġ������ ������ �̵�
	sNode TempNode = fPop(_stack);
	_loc->X = TempNode.X;
	_loc->Y = TempNode.Y;

	Maze[_loc->Y][_loc->X] = 2;
}


int main()
{
	//���ð� �ʱ�ȭ
	sLinkedStack * Stack = (sLinkedStack *)malloc(sizeof(sLinkedStack));
	Stack->mHead = NULL;
	Stack->mTop = NULL;
	Stack->mSize = 0;

	//������ ��ġ�� �ʱ�ȭ
	sUserLocation UserLocation;
	UserLocation.X = 0;
	UserLocation.Y = 0;

	while (1)
	{
		//�̵���ġ���� �Լ� ����
		SaveMovePoint(&UserLocation, Stack);
		//�̵��Լ� ����
		MovingPosition(&UserLocation, Stack);

		//���
		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				switch ((Maze[i][j]))
				{
				case 0:
					printf("��");
					break;
				case 1:
					printf("��");
					break;
				case 2:
					printf("��");
					break;
				}
			}
			printf("\n");
		}

		//���� ���̸� ����
		if ((UserLocation.X == 5) && (UserLocation.Y == 5))
		{
			break;
		}

		_sleep(500);
		system("cls");
	}

	//������ ���� ���� ���ؼ� �����
	for (int i = 0; i < Stack->mSize; i++)
	{
		fPop(Stack);
	}

	return 0;
}