#include "DoublyLinkedList.h"

int main( void )
{
    int   Count   = 0;
    Node* List1   = NULL;
	Node* List2 = NULL;
	Node* List3 = NULL;
    Node* NewNode = NULL;
    Node* Current = NULL;
	int num1, num2;

    /*  노드 5개 추가 */

	printf("List1 노드갯수, List2 노드갯수 입력 : ");
	scanf("%d %d", &num1, &num2);

	clock_t start, finish;
	double duraration;
	start = clock();

	for (int i = 0; i < num1 ; i++)
	{
		NewNode = DLL_CreateNode(i+10);
		DLL_AppendNode(&List1, NewNode);
	}

	for (int i = 0; i < num2; i++)
	{
		NewNode = DLL_CreateNode(i);
		DLL_AppendNode(&List2, NewNode);
	}

	ChainList(List1, List2, &List3);

    /*  리스트 출력 */
	Count = DLL_GetNodeCount(List1);
    for (int i = 0; i<Count; i++ )
    {
		Current = DLL_GetNodeAt(List1, i);
        printf( "List1[%d] : %d\n", i, Current->Data );
    }

	printf("\n");

	Count = DLL_GetNodeCount(List2);
	for (int i = 0; i<Count; i++)
	{
		Current = DLL_GetNodeAt(List2, i);
		printf("List2[%d] : %d\n", i, Current->Data);
	}

	printf("\n");

	Count = DLL_GetNodeCount(List3);
	for (int i = 0; i<Count; i++)
	{
		Current = DLL_GetNodeAt(List3, i);
		printf("List3[%d] : %d\n", i, Current->Data);
	}

	//BubbleSort(&List1);

	//Count = DLL_GetNodeCount(List1);
	//for (int i = 0; i<Count; i++)
	//{
	//	Current = DLL_GetNodeAt(List1, i);
	//	printf("List[%d] : %d\n", i, Current->Data);
	//}
	

    /*  모든 노드를 메모리에서 제거     */
    printf( "\nDestroying List...\n" );

	Count = DLL_GetNodeCount(List1);

    for (int i = 0; i<Count; i++ )
    {
		Current = DLL_GetNodeAt(List1, 0);

        if ( Current != NULL ) 
        {
			DLL_RemoveNode(&List1, Current);
            DLL_DestroyNode( Current );
        }
    }

	Count = DLL_GetNodeCount(List2);

	for (int i = 0; i<Count; i++)
	{
		Current = DLL_GetNodeAt(List2, 0);

		if (Current != NULL)
		{
			DLL_RemoveNode(&List2, Current);
			DLL_DestroyNode(Current);
		}
	}

	finish = clock();
	duraration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("%f 초입니다", duraration);
    return 0;
}