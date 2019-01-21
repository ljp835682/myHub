#include "LCRSTree.h"

LCRSNode* LCRS_CreateNode( ElementType NewData )			//����� �ʱ�ȭ
{
    LCRSNode* NewNode = (LCRSNode*)malloc( sizeof(LCRSNode) );
    NewNode->LeftChild    = NULL;
    NewNode->RightSibling = NULL;
    NewNode->Data = NewData;

    return NewNode;
}

void LCRS_DestroyNode( LCRSNode* Node )						//����� ����
{
    free(Node);
}

void LCRS_DestroyTree( LCRSNode* Root )						//Ʈ�� ����
{
    if ( Root->RightSibling != NULL )
        LCRS_DestroyTree( Root->RightSibling );

    if ( Root->LeftChild != NULL )
        LCRS_DestroyTree( Root->LeftChild );

    Root->LeftChild = NULL;									//�ش� ����� �ڽ� ������ �ʱ�ȭ
    Root->RightSibling = NULL;								//�ش� ����� ���� ������ �ʱ�ȭ

    LCRS_DestroyNode( Root );								//�ش� ��� ����
}

void LCRS_AddChildNode( LCRSNode* Parent, LCRSNode *Child)	//�ڽ� ��� �߰�
{
    if ( Parent->LeftChild == NULL )						//�ڽ� ��� �����Ͱ� �ƹ��͵� ����Ű�� �������
    {
        Parent->LeftChild = Child;							//�ڽ� ��� �����͸� �߰��� ����
    }
    else 
    {														//���� �ڽ� ��� �����Ͱ� ���𰡸� ����Ű���������
        LCRSNode* TempNode = Parent->LeftChild;				//�ڽ� ��带 �߰��� �θ��尡 ����Ű�� �ڽĳ�忡������ �����ؼ�
        while ( TempNode->RightSibling != NULL )			//�� �ڽ� ����� ������带 ��� ã�Ƴ���
            TempNode = TempNode->RightSibling;				//�ڽĳ���������� ���� ��� �����Ͱ� ���� ����Ű�� ��츦 ã�Ƴ���

        TempNode->RightSibling = Child;						//�ش� ����� ���� ��� �����Ϳ� �ش��带 ����Ű�� �Ѵ�
    }
}

void LCRS_PrintTree( LCRSNode* Node, int Depth )
{
    int i=0;				
    for ( i=0; i<Depth; i++ )								//���̸�ŭ �ٸ���
        printf(" ");

    printf("%c\n", Node->Data);								//������ ���

    if ( Node->LeftChild != NULL )							//�ڽĺ��� ���
        LCRS_PrintTree(Node->LeftChild, Depth+1);			//����Լ��� ���̸� �߰�

    if ( Node->RightSibling != NULL )						//������ �ڽ��� ���̻� ������� ���
        LCRS_PrintTree(Node->RightSibling, Depth);			//����Լ��� ���̴� �״��
}

