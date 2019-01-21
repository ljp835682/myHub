#include "LinkedList.h"

/*  ��� ���� */
Node* SLL_CreateNode(ElementType _Number, ElementType _Score)
{
    Node* NewNode = (Node*)malloc(sizeof(Node));

	NewNode->Number = _Number;  /*  �����͸� �����Ѵ�. */
	NewNode->Score = _Score;
    NewNode->NextNode = NULL; /*  ���� ��忡 ���� �����ʹ� NULL�� �ʱ�ȭ �Ѵ�. */

    return NewNode;/*  ����� �ּҸ� ��ȯ�Ѵ�. */
}

/*  ��� �Ҹ� */
void SLL_DestroyNode(Node* Node)
{
    free(Node);
}

/*  ��� �߰� */
void SLL_AppendNode(Node** Head, Node* NewNode)
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
    }
}

/*  ��� ���� */
void SLL_InsertAfter(Node* Current, Node* NewNode)
{
    NewNode->NextNode = Current->NextNode;
    Current->NextNode = NewNode;
}

void  SLL_InsertNewHead(Node** Head, Node* NewHead)
{
    if ( Head == NULL )
    {
        (*Head) = NewHead;    
    }
    else
    {
        NewHead->NextNode = (*Head);
        (*Head) = NewHead;
    }
}

/*  ��� ���� */
void SLL_RemoveNode(Node** Head, Node* Remove)
{
    if ( *Head == Remove )
    {
        *Head = Remove->NextNode;
    }
    else
    {
        Node* Current = *Head;
        while ( Current != NULL && Current->NextNode != Remove )
        {
            Current = Current->NextNode;
        }

        if ( Current != NULL )
            Current->NextNode = Remove->NextNode;
    }
}

/*  ��� Ž�� */
Node* SLL_GetNodeAt(Node* Head, int Location)
{
    Node* Current = Head;

    while ( Current != NULL && (--Location) >= 0)
    {
        Current = Current->NextNode;
    }

    return Current;
}

/* ��� �˻� */
Node* SLL_SearchNode(Node* Head, int _Number)
{
	Node* Current = Head;

	while (1)
	{
		if (Current->Number == _Number)		//�� ���ڿ� ������ ��ȣ�� ���� ��尡 ������
		{	
			return Current;					//�ش� ��� ����
		}

		else if (Current->NextNode == NULL)	//���ϱ����ͼ� �߰� ���ϸ�
		{
			return NULL;					//�� ����
		}

		else
		{
			Current = Current->NextNode;	//�׿ܿ� ���� ���� �̵�
		}
	}
}


/*  ��� �� ���� */
int SLL_GetNodeCount(Node* Head)
{
    int   Count = 0;
    Node* Current = Head;

    while ( Current != NULL )
    {
        Current = Current->NextNode;
        Count++;
    }

    return Count;
}
