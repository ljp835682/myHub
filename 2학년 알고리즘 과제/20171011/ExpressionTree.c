#include "ExpressionTree.h"

ETNode* ET_CreateNode( ElementType NewData )				//�ʱ�ȭ
{
    ETNode* NewNode = (ETNode*)malloc( sizeof(ETNode) );
    NewNode->Left    = NULL;
    NewNode->Right   = NULL;
    NewNode->Data    = NewData;

    return NewNode;
}

void ET_DestroyNode( ETNode* Node )							//����� ����
{
    free(Node);
}

void ET_DestroyTree( ETNode* Root )							//Ʈ���� ����
{
    if ( Root == NULL )										//���� �ڽ��� ����Ʈ���� �ƴ϶��
        return;												//�׳� ����

    ET_DestroyTree( Root->Left );							//����Լ��� ������ ��尡 ����Ʈ����� ����
    ET_DestroyTree( Root->Right );							//����Լ��� �������� ��尡 ����Ʈ����� ����
    ET_DestroyNode( Root );									//�ڽ��� ����
}

void ET_PreorderPrintTree( ETNode* Node )					//������ ���
{
    if ( Node == NULL )
        return;												//���� �� ���� ����

    printf( " %c", Node->Data );							//ù��°�� �ڽ��� �����͸� ���� ���

    ET_PreorderPrintTree( Node->Left );						//�ι�°�� ���� ����� �����͸� ���
    ET_PreorderPrintTree( Node->Right );					//����°�� ������ ����� �����͸� ���
}															//��������� ����� ��ū�� �տ����� �� ������ �ΰ��� ����� ���� ���Եɰ�  

void ET_InorderPrintTree( ETNode* Node )					//������ ���
{
    if ( Node == NULL )
        return;												//���� �� ���� ����
    
    printf( "(" );
    ET_InorderPrintTree( Node->Left );						//ù��°�� ������ �����͸� ����Լ��� ���

    printf( "%c", Node->Data );								//�ι�°�� �ڽ��� �����͸� ���

    ET_InorderPrintTree( Node->Right );						//����°�� �������� �����͸� ����Լ��� ���
    printf( ")" );				
}															//��������� ����� ��ū�� ���� ���ʰ� �����ʿ� ����� ���� ���Եɰ�

void ET_PostorderPrintTree( ETNode* Node )					//������ ���
{
    if ( Node == NULL )										//���� �� ���� ����
        return;
    
    ET_PostorderPrintTree( Node->Left );					//ù��°�� ������ ����� �����͸� ���� ���
    ET_PostorderPrintTree( Node->Right );					//�ι�°�� �������� ����� �����͸� ���
    printf( " %c", Node->Data );							//����°�� �ڽ��� �����͸� ���
}

void ET_BuildExpressionTree( char* PostfixExpression, ETNode** Node )		//����Ʈ�� ���� �Լ�
{
    ETNode* NewNode = NULL;
    int  len        = strlen( PostfixExpression );			//���� ����
    char Token      = PostfixExpression[ len -1 ];			//�ι��ڰ� �ƴ� ������ ����
    PostfixExpression[ len-1 ] = '\0';						//��ū���� ���� ������ ��ġ�� �ι��� ����

    switch ( Token ) 
    {
        /*  �������� ��� */
        case '+': case '-': case '*': case '/':				
            (*Node) = ET_CreateNode( Token );			//��ū�� �ڽ��� ���ܾ���
            ET_BuildExpressionTree( PostfixExpression, &(*Node)->Right );	//ù��°�� �����ʿ� ���� ��ū�� �ڽ����� 
            ET_BuildExpressionTree( PostfixExpression, &(*Node)->Left  );	//�ι�°�� ���ʿ� ���� ��ū�� �ڽ�����
            break;

        /*  �ǿ������� ��� */
        default :
            (*Node) = ET_CreateNode( Token);			//������ ��� �ڽ��� �������� �����Ƿ� �׳� ������
            break;
    }
}

double ET_Evaluate( ETNode* Tree )						//Ʈ�� ��� �Լ�
{
    char Temp[2];
    
    double Left   = 0;									//������ Ʈ������ ���ɰ�
    double Right  = 0;									//������ Ʈ������ ���ɰ�
    double Result = 0;									//�����

    if ( Tree == NULL )									//����Լ��� ���� ���ϱ����� ���ϰ�� �Լ�����
        return 0;

    switch ( Tree->Data ) 
    {
        /*  �������� ��� */
        case '+': case '-': case '*': case '/':			//����� �ؾ���
            Left  = ET_Evaluate( Tree->Left );			//���� Ʈ���� �ִٸ� ���� ����ϰ�
            Right = ET_Evaluate( Tree->Right );			//������ Ʈ���� �ִٸ� ����Ͽ� ���� ����

                 if ( Tree->Data == '+' ) Result = Left + Right;
            else if ( Tree->Data == '-' ) Result = Left - Right;
            else if ( Tree->Data == '*' ) Result = Left * Right;
            else if ( Tree->Data == '/' ) Result = Left / Right;		//�´� ����� ����

            break;

        /*  �ǿ������� ��� */
        default :
            memset(Temp, 0, sizeof(Temp));		//��ĭ���� �ٲ�ΰ�
            Temp[0] = Tree->Data;				//�����͸� �־
            Result = atof(Temp);				//���ڸ� �Ǽ���
            break;
    }

    return Result;				//����� ����
}