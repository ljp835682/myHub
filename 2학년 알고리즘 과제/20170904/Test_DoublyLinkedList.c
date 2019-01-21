#include "DoublyLinkedList.h"

int main( void )
{
    int   Count   = 0;
    Node* List    = NULL;
    Node* NewNode = NULL;
    Node* Current = NULL;

    /*  노드 5개 추가 */

	NewNode = DLL_CreateNode(4);
	DLL_AppendNode(&List, NewNode);
	NewNode = DLL_CreateNode(2);
	DLL_AppendNode(&List, NewNode);
	NewNode = DLL_CreateNode(1);
	DLL_AppendNode(&List, NewNode);
	NewNode = DLL_CreateNode(3);
	DLL_AppendNode(&List, NewNode);
	NewNode = DLL_CreateNode(5);
	DLL_AppendNode(&List, NewNode);


    /*  리스트 출력 */
    Count = DLL_GetNodeCount( List );
    for (int i = 0; i<Count; i++ )
    {
        Current = DLL_GetNodeAt( List, i );
        printf( "List[%d] : %d\n", i, Current->Data );
    }

	printf("\n\n");

	BubbleSort(&List);

	Count = DLL_GetNodeCount(List);
	for (int i = 0; i<Count; i++)
	{
		Current = DLL_GetNodeAt(List, i);
		printf("List[%d] : %d\n", i, Current->Data);
	}

	///* 테일 탐색 */

	//Current = List;		//검색용 노드에 헤드 대입

	//while (1)
	//{
	//	if (Current->NextNode == NULL)		//만약 검색용 노드의 다음이 널이면
	//	{	
	//		break;							//반복문 탈출
	//	}

	//	else
	//	{
	//		Current = Current->NextNode;	//그외엔 검색용 노드를 다음노드로 바꿈
	//	}
	//}

	///* 역 출력 */

	//Count = DLL_GetNodeCount(List);			//가시성을 위해 총 노드의 갯수 구함

	//while (1)
	//{
	//	printf("%d번째로 연결된 노드 : %d\n", Count--, Current->Data);	//검색용 노드가 가리키는 노드속 데이터를 출력

	//	if (Current->PrevNode == NULL)		//만약 검색용 노드의 이전 노드가 널일경우
	//	{
	//		break;							//반복문 탈출
	//	}

	//	else
	//	{
	//		Current = Current->PrevNode;	//그외엔 검색용  노드를 이전노드로 바꿈
	//	}
	//}


    ///*  리스트의 세번째 칸 뒤에 노드 삽입 */
    //printf( "\nInserting 3000 After [2]...\n\n" );

    //Current = DLL_GetNodeAt( List, 2 );
    //NewNode = DLL_CreateNode( 3000 );
    //DLL_InsertAfter( Current, NewNode );

    ///*  리스트 출력 */
    //Count = DLL_GetNodeCount( List );
    //for ( i = 0; i<Count; i++ )
    //{
    //    Current = DLL_GetNodeAt( List, i );
    //    printf( "List[%d] : %d\n", i, Current->Data );
    //}

    /*  모든 노드를 메모리에서 제거     */
    printf( "\nDestroying List...\n" );

    Count = DLL_GetNodeCount(List);

    for (int i = 0; i<Count; i++ )
    {
        Current = DLL_GetNodeAt( List, 0 );

        if ( Current != NULL ) 
        {
            DLL_RemoveNode( &List, Current );
            DLL_DestroyNode( Current );
        }
    }

    return 0;
}