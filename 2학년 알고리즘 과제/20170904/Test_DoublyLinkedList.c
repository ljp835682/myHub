#include "DoublyLinkedList.h"

int main( void )
{
    int   Count   = 0;
    Node* List    = NULL;
    Node* NewNode = NULL;
    Node* Current = NULL;

    /*  ��� 5�� �߰� */

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


    /*  ����Ʈ ��� */
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

	///* ���� Ž�� */

	//Current = List;		//�˻��� ��忡 ��� ����

	//while (1)
	//{
	//	if (Current->NextNode == NULL)		//���� �˻��� ����� ������ ���̸�
	//	{	
	//		break;							//�ݺ��� Ż��
	//	}

	//	else
	//	{
	//		Current = Current->NextNode;	//�׿ܿ� �˻��� ��带 �������� �ٲ�
	//	}
	//}

	///* �� ��� */

	//Count = DLL_GetNodeCount(List);			//���ü��� ���� �� ����� ���� ����

	//while (1)
	//{
	//	printf("%d��°�� ����� ��� : %d\n", Count--, Current->Data);	//�˻��� ��尡 ����Ű�� ���� �����͸� ���

	//	if (Current->PrevNode == NULL)		//���� �˻��� ����� ���� ��尡 ���ϰ��
	//	{
	//		break;							//�ݺ��� Ż��
	//	}

	//	else
	//	{
	//		Current = Current->PrevNode;	//�׿ܿ� �˻���  ��带 �������� �ٲ�
	//	}
	//}


    ///*  ����Ʈ�� ����° ĭ �ڿ� ��� ���� */
    //printf( "\nInserting 3000 After [2]...\n\n" );

    //Current = DLL_GetNodeAt( List, 2 );
    //NewNode = DLL_CreateNode( 3000 );
    //DLL_InsertAfter( Current, NewNode );

    ///*  ����Ʈ ��� */
    //Count = DLL_GetNodeCount( List );
    //for ( i = 0; i<Count; i++ )
    //{
    //    Current = DLL_GetNodeAt( List, i );
    //    printf( "List[%d] : %d\n", i, Current->Data );
    //}

    /*  ��� ��带 �޸𸮿��� ����     */
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