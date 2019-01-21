#include "ArrayStack.h"

void  AS_CreateStack(ArrayStack** Stack, int Capacity)
{
    /*  스택을 자유저장소에 생성 */
    (*Stack)           = (ArrayStack*)malloc(sizeof(ArrayStack));

    /*  입력된 Capacity만큼의 노드를 자유저장소에 생성 */
    (*Stack)->Nodes    = (Node*)malloc(sizeof(Node)*Capacity);

    /*  Capacity 및 Top 초기화 */
    (*Stack)->Capacity = Capacity;
    (*Stack)->Top = 0;
}

void AS_DestroyStack(ArrayStack* Stack)
{
    /*  노드를 자유 저장소에서 해제 */
    free(Stack->Nodes);

    /*  스택을 자유 저장소에서 해제 */
    free(Stack);
}

void AS_Push(ArrayStack* Stack, ElementType Data)		//데이터를 넣는 함수
{
    int Position = Stack->Top;					//위치를 현재 탑의 위로 정함

    Stack->Nodes[Position].Data = Data;			//그 위치의 데이터에 넣을값을 넣음
    Stack->Top++;								//데이터를 넣었으므로 탑위의 빈칸을 가르킴
}

ElementType AS_Pop(ArrayStack* Stack)			//데이터를 빼는 함수
{
    int Position = --( Stack->Top );			//현재 가장 높은 위치의 노드의 위치를 알아낸후 영구적으로 Top을 줄임

    return Stack->Nodes[Position].Data;			//그 노드의 데이터를 리턴
}

ElementType AS_Top(ArrayStack* Stack)			//현재 탑을 알아내는 함수
{
    int Position = Stack->Top - 1;				//현재 가장 높은 위치의 노드의 위치를 알아낸후 임시적으로 Top을 줄임

    return Stack->Nodes[Position].Data;			//그 노드의 데이터를 리턴 
}

int AS_GetSize(ArrayStack* Stack)				//스택의 크기를 알아내는 함수
{
    return Stack->Top;							//스택의 가장 높은 노드의 위치를 리턴
}

int AS_IsEmpty(ArrayStack* Stack)				//스택을 비우는 함수
{
    return (Stack->Top == 0);					//스택의 가장위를 나타내는 변수를 0으로 강제변환
}