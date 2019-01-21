#include "DoublyLinkedList.h"

/*  ��� ���� */
Node* DLL_CreateNode( ElementType NewData )
{
    Node* NewNode = (Node*)malloc(sizeof(Node));

    NewNode->Data = NewData;
    NewNode->PrevNode = NULL;
    NewNode->NextNode = NULL;

    return NewNode;
}

/*  ��� �Ҹ� */
void DLL_DestroyNode( Node* Node )
{
    free(Node);
}

/*  ��� �߰� */
void DLL_AppendNode( Node** Head, Node* NewNode )
{
    /*  ��� ��尡 NULL�̶�� ���ο� ��尡 Head */
    if ( (*Head) == NULL ) 
    {
        *Head = NewNode;
    } 
    else
    {
        /*  ������ ã�� NewNode�� �����Ѵ�. */
        Node* Tail = (*Head);
        while ( Tail->NextNode != NULL )
        {
            Tail = Tail->NextNode;
        }

        Tail->NextNode = NewNode;
        NewNode->PrevNode = Tail; /*  ������ ������ ���ο� ������ PrevNode�� ����Ų��. */
    }
}

/*  ��� ���� */
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

/*  ��� ���� */
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

/*  ��� Ž�� */
Node* DLL_GetNodeAt( Node* Head, int Location )
{
    Node* Current = Head;

    while ( Current != NULL && (--Location) >= 0)
    {
        Current = Current->NextNode;
    }

    return Current;
}

/*  ��� �� ���� */
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
	Node * CurrentA = List1Head->NextNode;						//����Ʈ1�� ��带 �����ϰ� ����Ʈ1�� ����� ���� ��带 ���� 
	Node * CurrentB = List2Head;								//����Ʈ2�� ��忡������ ����

	*List3Head = DLL_CreateNode(List1Head->Data);				//����Ʈ3�� ����Ʈ1�� ����� �����Ϳ� ���� ���� �������� �������� ����

	Node * CurrentC = *List3Head;								//����Ʈ3�� ��忡������ ����
	
	int Flag = 0;												//A�� B�� �����ư��鼭 �����Ҽ��հ� �ϵ��� �ϴ� ����

	while (1)
	{
		if (Flag == 0)											//A�� ���� �̹� ������ �������־����Ƿ� B���� ����
		{
			DLL_AppendNode(&CurrentC, DLL_CreateNode(CurrentB->Data));		//����Ʈ3�� CurrentC�� ����Ű�°��� ����ĭ�� CurrentB�� ����Ű�� ����� ������ �� ���� ���� ���������� ��������			
			Flag = 1;											//������ �������� A�� ����
			
			if (CurrentA == NULL)								//���� CurrentA�� NULL�ϰ��
			{
				Flag = 0;										//������ B
			}

			CurrentB = CurrentB->NextNode;						//CurrentB�� �������� ����
		}

		if (Flag == 1)											//B
		{
			DLL_AppendNode(&CurrentC, DLL_CreateNode(CurrentA->Data));		//����Ʈ3�� CurrentC�� ����Ű�°��� ����ĭ�� CurrentA�� ����Ű�� ����� ������ �� ���� ���� ���������� ��������			
			Flag = 0;											//������ �������� B�� ����

			if (CurrentB == NULL)								//���� CurrentB�� NULL�ϰ��
			{
				Flag = 1;										//������ A
			}

			CurrentA = CurrentA->NextNode;						//A�� ���� ���� ����
		}

		CurrentC = CurrentC->NextNode;							//CurrentC�� �������� ����

		if ((CurrentA == NULL) && (CurrentB == NULL))			//���� A�� B�� �Ѵ� NULL�ϰ��
		{
			break;												//�ݺ��� ����
		}
	}
}
