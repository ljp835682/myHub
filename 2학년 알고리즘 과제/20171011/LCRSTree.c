#include "LCRSTree.h"

LCRSNode* LCRS_CreateNode( ElementType NewData )			//노드의 초기화
{
    LCRSNode* NewNode = (LCRSNode*)malloc( sizeof(LCRSNode) );
    NewNode->LeftChild    = NULL;
    NewNode->RightSibling = NULL;
    NewNode->Data = NewData;

    return NewNode;
}

void LCRS_DestroyNode( LCRSNode* Node )						//노드의 삭제
{
    free(Node);
}

void LCRS_DestroyTree( LCRSNode* Root )						//트리 삭제
{
    if ( Root->RightSibling != NULL )
        LCRS_DestroyTree( Root->RightSibling );

    if ( Root->LeftChild != NULL )
        LCRS_DestroyTree( Root->LeftChild );

    Root->LeftChild = NULL;									//해당 노드의 자식 포인터 초기화
    Root->RightSibling = NULL;								//해당 노드의 형제 포인터 초기화

    LCRS_DestroyNode( Root );								//해당 노드 삭제
}

void LCRS_AddChildNode( LCRSNode* Parent, LCRSNode *Child)	//자식 노드 추가
{
    if ( Parent->LeftChild == NULL )						//자식 노드 포인터가 아무것도 가르키지 않을경우
    {
        Parent->LeftChild = Child;							//자식 노드 포인터를 추가할 노드로
    }
    else 
    {														//만약 자식 노드 포인터가 무언가를 가르키고있을경우
        LCRSNode* TempNode = Parent->LeftChild;				//자식 노드를 추가할 부모노드가 가르키는 자식노드에서부터 시작해서
        while ( TempNode->RightSibling != NULL )			//그 자식 노드의 형제노드를 계속 찾아내서
            TempNode = TempNode->RightSibling;				//자식노드포인터의 형제 노드 포인터가 널을 가르키는 경우를 찾아내서

        TempNode->RightSibling = Child;						//해당 노드의 형제 노드 포인터에 해당노드를 가르키게 한다
    }
}

void LCRS_PrintTree( LCRSNode* Node, int Depth )
{
    int i=0;				
    for ( i=0; i<Depth; i++ )								//깊이만큼 줄맞춤
        printf(" ");

    printf("%c\n", Node->Data);								//데이터 출력

    if ( Node->LeftChild != NULL )							//자식부터 출력
        LCRS_PrintTree(Node->LeftChild, Depth+1);			//재귀함수로 깊이를 추가

    if ( Node->RightSibling != NULL )						//형제는 자식이 더이상 없을경우 출력
        LCRS_PrintTree(Node->RightSibling, Depth);			//재귀함수로 깊이는 그대로
}

