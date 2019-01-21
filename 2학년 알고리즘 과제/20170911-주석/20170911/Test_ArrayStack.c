#include "ArrayStack.h"

int main( void )
{		
    ArrayStack* Stack = NULL;			//스택 선언

    AS_CreateStack(&Stack, 10);			//총용량을 10으로 정함
    
    AS_Push(Stack, 3);					//첫번째 데이터입력 현재 탑 3
    AS_Push(Stack, 37);					//두번째 데이터입력 현재 탑 37
    AS_Push(Stack, 11);					//세번째 데이터입력 현재 탑 11
    AS_Push(Stack, 12);					//네번째 데이터입력 현재 탑 12

    printf( "Capacity: %d, Size: %d, Top: %d\n\n",					//출력
        Stack->Capacity, AS_GetSize(Stack), AS_Top(Stack) );		//스택의 용량,스택의 크기,현재 스택의 탑

	for (int i = 0; i < 4; i++)			
	{
		if (AS_IsEmpty(Stack))			//스택에 데이터가 들어있지 않을경우
			break;						//반복문 종료

		printf("Popped: %d, ", AS_Pop(Stack));				//맨 위의 데이터를 알아내서 출력후 뺌

		if (!AS_IsEmpty(Stack))								//만약 스택이 빈칸이 아닐경우
		{
			printf("Current Top: %d\n", AS_Top(Stack));		//이전 탑의 데이터 출력
		}

		else
		{
			printf("Stack Is Empty.\n");					//스택이 빈칸이면 메세지 출력
		}
	}

    AS_DestroyStack(Stack);									//스택 반환

    return 0;
}