#include "DoublyLinkedList.h"

/*  노드 생성 */
Node* DLL_CreateNode( ElementType NewData )
{
    Node* NewNode = (Node*)malloc(sizeof(Node));

    NewNode->Data = NewData;
    NewNode->PrevNode = NULL;
    NewNode->NextNode = NULL;

    return NewNode;
}

/*  노드 소멸 */
void DLL_DestroyNode( Node* Node )
{
    free(Node);
}

/*  노드 추가 */
void DLL_AppendNode( Node** Head, Node* NewNode )
{
    /*  헤드 노드가 NULL이라면 새로운 노드가 Head */
    if ( (*Head) == NULL ) 
    {
        *Head = NewNode;
    } 
    else
    {
        /*  테일을 찾아 NewNode를 연결한다. */
        Node* Tail = (*Head);
        while ( Tail->NextNode != NULL )
        {
            Tail = Tail->NextNode;
        }

        Tail->NextNode = NewNode;
        NewNode->PrevNode = Tail; /*  기존의 테일을 새로운 테일의 PrevNode가 가리킨다. */
    }
}

/*  노드 삽입 */
void DLL_InsertAfter( Node* Current, Node* NewNode )
{
    NewNode->NextNode = Current->NextNode;
    NewNode->PrevNode = Current;

    if ( Current->NextNode != NULL )
    {
        Current->NextNode->PrevNode = NewNode;
        Current->NextNode = NewNode;
    }
}

/*  노드 제거 */
void DLL_RemoveNode( Node** Head, Node* Remove )
{
    if ( *Head == Remove )
    {
        *Head = Remove->NextNode;
        if ( (*Head) != NULL )
            (*Head)->PrevNode = NULL;
        
        Remove->PrevNode = NULL;
        Remove->NextNode = NULL;
    }
    else
    {
        Node* Temp = Remove;

        if ( Remove->PrevNode != NULL )
            Remove->PrevNode->NextNode = Temp->NextNode;

        if ( Remove->NextNode != NULL )
            Remove->NextNode->PrevNode = Temp->PrevNode;

        Remove->PrevNode = NULL;
        Remove->NextNode = NULL;
    }    
}

/*  노드 탐색 */
Node* DLL_GetNodeAt( Node* Head, int Location )
{
    Node* Current = Head;

    while ( Current != NULL && (--Location) >= 0)
    {
        Current = Current->NextNode;
    }

    return Current;
}

/*  노드 수 세기 */
int DLL_GetNodeCount( Node* Head )
{
    unsigned int  Count = 0;
    Node*         Current = Head;

    while ( Current != NULL )
    {
        Current = Current->NextNode;
        Count++;
    }

    return Count;
}

void BubbleSort(Node ** Head)
{
	Node * Temp;

	for (int i = DLL_GetNodeCount(*Head); i > 0; i--)
	{
		for (Node * p = *Head; p->NextNode != NULL; p = p->NextNode)
		{
			
			if (p->Data > p->NextNode->Data)
			{
				Temp = p->NextNode;

				p->NextNode = p->NextNode->NextNode;
				p->NextNode->PrevNode = p;
				
				if (p->PrevNode == NULL)
				{
					*Head = Temp;
					Temp->PrevNode = NULL;
				}

				else
				{
					p->PrevNode->NextNode = Temp;
					Temp->PrevNode = p->PrevNode;
				}

				p->PrevNode = Temp;
				Temp->NextNode = p;
			}
		}
	}
}

void PrintNode( Node* _Node )
{
    if ( _Node->PrevNode == NULL )
        printf("Prev: NULL");
    else
        printf("Prev: %d", _Node->PrevNode->Data);

    printf(" Current: %d ", _Node->Data);

    if ( _Node->NextNode == NULL )
        printf("Next: NULL\n");
    else
        printf("Next: %d\n", _Node->NextNode->Data);
}

void ChainList(Node * List1Head, Node * List2Head, Node ** List3Head)
{
	Node * CurrentA = List1Head->NextNode;						//리스트1의 헤드를 생략하고 리스트1의 헤드의 다음 노드를 지정 
	Node * CurrentB = List2Head;								//리스트2의 헤드에서부터 지정

	*List3Head = DLL_CreateNode(List1Head->Data);				//리스트3에 리스트1의 헤드의 데이터와 같은 값일 가지도록 깊은복사 생성

	Node * CurrentC = *List3Head;								//리스트3의 헤드에서부터 지정
	
	int Flag = 0;												//A와 B를 번갈아가면서 지정할수잇게 하도록 하는 변수

	while (1)
	{
		if (Flag == 0)											//A의 헤드는 이미 위에서 지정해주었으므로 B부터 시작
		{
			DLL_AppendNode(&CurrentC, DLL_CreateNode(CurrentB->Data));		//리스트3에 CurrentC가 가르키는것의 다음칸에 CurrentB가 가르키는 노드의 데이터 와 같은 값을 가진것으로 깊은복사			
			Flag = 1;											//다음에 이을것을 A로 지정
			
			if (CurrentA == NULL)								//만약 CurrentA가 NULL일경우
			{
				Flag = 0;										//다음도 B
			}

			CurrentB = CurrentB->NextNode;						//CurrentB는 다음노드로 지정
		}

		if (Flag == 1)											//B
		{
			DLL_AppendNode(&CurrentC, DLL_CreateNode(CurrentA->Data));		//리스트3에 CurrentC가 가르키는것의 다음칸에 CurrentA가 가르키는 노드의 데이터 와 같은 값을 가진것으로 깊은복사			
			Flag = 0;											//다음에 이을것을 B로 지정

			if (CurrentB == NULL)								//만약 CurrentB가 NULL일경우
			{
				Flag = 1;										//다음도 A
			}

			CurrentA = CurrentA->NextNode;						//A는 다음 노드로 지정
		}

		CurrentC = CurrentC->NextNode;							//CurrentC를 다음노드로 이행

		if ((CurrentA == NULL) && (CurrentB == NULL))			//만약 A와 B가 둘다 NULL일경우
		{
			break;												//반복문 종료
		}
	}
}
