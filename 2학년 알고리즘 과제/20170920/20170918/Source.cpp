#include "LinkedStack.h"

//미로
int Maze[6][6] = {	{ 2, 1, 1, 1, 1, 0 },
					{ 0, 0, 0, 0, 0, 0 },
					{ 1, 1, 0, 1, 1, 1 },
					{ 1, 1, 0, 0, 0, 1 },
					{ 1, 1, 0, 1, 0, 1 },
					{ 0, 0, 0, 1, 0, 0 }, };

//유저의 위치
typedef struct structUserLocation
{
	int X;
	int Y;
}sUserLocation;

//갈수있는 위치를 저장하는 함수
void SaveMovePoint(sUserLocation * _loc, sLinkedStack * _stack)
{
	//(우선도) 방향
	//(4) 오른쪽
	if (_loc->X + 1 < 6)
	{
		if (Maze[_loc->Y][_loc->X + 1] == 0)
		{

			fPush(_stack, _loc->X + 1, _loc->Y);
		}
	}

	//(3) 왼쪽
	if (_loc->X - 1 >= 0)
	{
		if (Maze[_loc->Y][_loc->X - 1] == 0)
		{
			fPush(_stack, _loc->X - 1, _loc->Y);
		}
	}

	//(2) 아래쪽
	if (_loc->Y + 1 < 6)
	{
		if (Maze[_loc->Y + 1][_loc->X] == 0)
		{
			fPush(_stack, _loc->X, _loc->Y + 1);
		}
	}

	//(1) 위쪽
	if (_loc->Y - 1 < 0)
	{
		if (Maze[_loc->Y - 1][_loc->X] == 0)
		{
			fPush(_stack, _loc->X, _loc->Y - 1);
		}
	}
}

//유저의 이동함수
void MovingPosition(sUserLocation * _loc, sLinkedStack * _stack)
{	
	//팝해서 맨위의 노드를 꺼내 그 노드에 저장된 위치값으로 유저가 이동
	sNode TempNode = fPop(_stack);
	_loc->X = TempNode.X;
	_loc->Y = TempNode.Y;

	Maze[_loc->Y][_loc->X] = 2;
}


int main()
{
	//스택과 초기화
	sLinkedStack * Stack = (sLinkedStack *)malloc(sizeof(sLinkedStack));
	Stack->mHead = NULL;
	Stack->mTop = NULL;
	Stack->mSize = 0;

	//유저의 위치와 초기화
	sUserLocation UserLocation;
	UserLocation.X = 0;
	UserLocation.Y = 0;

	while (1)
	{
		//이동위치저장 함수 실행
		SaveMovePoint(&UserLocation, Stack);
		//이동함수 실행
		MovingPosition(&UserLocation, Stack);

		//출력
		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				switch ((Maze[i][j]))
				{
				case 0:
					printf("□");
					break;
				case 1:
					printf("■");
					break;
				case 2:
					printf("◇");
					break;
				}
			}
			printf("\n");
		}

		//만약 골이면 종료
		if ((UserLocation.X == 5) && (UserLocation.Y == 5))
		{
			break;
		}

		_sleep(500);
		system("cls");
	}

	//쓰래기 노드들 전부 팝해서 사라짐
	for (int i = 0; i < Stack->mSize; i++)
	{
		fPop(Stack);
	}

	return 0;
}