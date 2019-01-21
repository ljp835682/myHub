#include "LinkedList.h"

/*
시작하면
1. 입력 (맨뒤로)
2. 삭제
3. 수정
4. 삽입
5. 출력
6. 종료

1번선택시
번호와 점수를 입력받음

2번선택시
번호를 검색해서 삭제

4번선택시
번호를 검색해서 그 뒤에 추가
*/

int MenuPrint()			//메뉴 출력용 함수입니다
{
	int Choice = 0;

	printf("1. 입력\n");
	printf("2. 삭제\n");
	printf("3. 수정\n");
	printf("4. 삽입\n");
	printf("5. 출력\n");
	printf("6. 종료\n");
	printf("선택 : ");		//문자출력후
	scanf("%d", &Choice);	//사용자에게 입력을 받고
	return Choice;			//입력받은 숫자를 외부로 보냅니다
}
int main( void )
{
    int   Count   = 0;		//노드의 총갯수를 셀 정수형 변수
    Node* List    = NULL;	//리스트의 헤드
    Node* Current = NULL;	//새로운 노드를 만들 공간
	Node * FindNode = NULL;
	int flag = 0;			//종료 플래그
	int Number;				//번호를 입력시킬 정수형 변수
	int Score;				//점수를 입력시킬 정수형 변수

	while (1)
	{
		Number = 0;	
		Score = 0;			//초기화

		switch (MenuPrint())		//메뉴에서 입력받은 숫자로 switch문을 실행합니다
		{
		case 1:						
			printf("번호 : ");
			scanf("%d", &Number);
			printf("점수 : ");
			scanf("%d", &Score);		//번호와 점수를 입력받습니다

			Current = SLL_CreateNode(Number, Score);		//새로운 노드를 만들어내고
			SLL_AppendNode(&List, Current);					//맨끝에 추가시킵니다
			break;
		case 2:
			printf("삭제할 번호 : ");						
			scanf("%d", &Number);							//삭제할 번호를 입력받습니다

			FindNode = SLL_SearchNode(List, Number);		//입력한 번호가있는지 검색합니다

			if (FindNode != NULL)								//널이아니라면
			{
				SLL_RemoveNode(&List, FindNode);				//삭제합니다
			}

			else
			{
				printf("없는 번호\n");							//널이면 입력한 번호가 없는상황입니다
			}

			break;
		case 3:
			printf("수정할 번호 : ");								
			scanf("%d", &Number);									//수정할 번호를 입력받습니다

			FindNode = SLL_SearchNode(List, Number);				//입력한 번호가 있는지 검색합니다

			if (FindNode != NULL)									//널이아니라면
			{
				printf("수정하기 전 점수 : %d\n", FindNode->Score);	//수정하기전 점수를 보여줍니다
				printf("점수 입력 : ");
				scanf("%d", &FindNode->Score);						//새로운 점수를 입력받습니다
			}

			else
			{
				printf("없는 번호\n");								//널이면 입력한 번호가 없는상황입니다
			}

			break;
		case 4:
			printf("기준이 될 번호 : ");
			scanf("%d", &Number);								//삽입하는데 기준이될 번호를 입력받습니다

			FindNode = SLL_SearchNode(List, Number);			//입력한 번호가 있는지 찾아봅니다

			if (FindNode != NULL)								//널이 아니라면
			{
				printf("번호 : ");
				scanf("%d", &Number);
				printf("점수 : ");
				scanf("%d", &Score);							//점수를 입력받고

				Current = SLL_CreateNode(Number, Score);		//새로운 노드를 만든후
				Current->NextNode = FindNode->NextNode;			//그노드의 다음노드를 기준이 되는 노드의 다음노드로 설정해주고
				FindNode->NextNode = Current;					//기준이 되는 노드의 다음노드를 새로만든 노드로 설정해줍니다
			}

			else
			{
				printf("없는 번호\n");
			}
			
			break;
		case 5:
			Count = SLL_GetNodeCount(List);						//모든 노드의 숫자를 센후
			for (int i = 0; i<Count; i++ )						//모든 노드의 숫자만큼 반복시켜서
			{
			    Current = SLL_GetNodeAt(List, i);
				printf("%d번의 점수 : %d\n", Current->Number, Current->Score);		//모두 출력시킵니다
			}
			break;
		case 6:
			flag = 1;					//종료플래그입니다
			break;
		}

		if (flag == 1)					//종료 조건문입니다
		{
			break;						//반복문을 나가 프로그램이 종료하게됩니다
		}
	}
    return 0;
}


