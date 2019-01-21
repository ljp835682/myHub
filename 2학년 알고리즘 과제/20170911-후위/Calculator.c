#include "Calculator.h"

char NUMBER[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.' };

int IsNumber( char Cipher )
{
    int i = 0;
    int ArrayLength = sizeof(NUMBER);

    for ( i=0; i<ArrayLength; i++ )
    {
        if ( Cipher == NUMBER[i] )
            return 1;
    }

    return 0;
}

unsigned int GetNextToken( char* Expression, char* Token, int* TYPE )
{
    unsigned int i = 0;
    
    for ( i=0 ; 0 != Expression[i]; i++ )
    {
        Token[i] = Expression[i];

        if ( IsNumber( Expression[i] ) == 1 )
        {            
            *TYPE = OPERAND;

            if ( IsNumber(Expression[i+1]) != 1 )
                break;
        }
        else
        {
            *TYPE = Expression[i];
            break;            
        }
    }

    Token[++i] = '\0';
    return i;
}

int GetPriority(char Operator, int InStack)
{
    int Priority = -1;

    switch (Operator)
    {
    case LEFT_PARENTHESIS:
        if ( InStack )
            Priority = 3;
        else
            Priority = 0;
        break;

    case MULTIPLY:
    case DIVIDE:
        Priority = 1;
        break;

    case PLUS:
    case MINUS:
        Priority = 2;
        break;
    }

    return Priority;
}

int IsPrior( char OperatorInStack, char OperatorInToken )
{
    return ( GetPriority(OperatorInStack, 1) > GetPriority(OperatorInToken, 0) );
}

void GetPostfix( char* InfixExpression, char* PostfixExpression )		//������,������
{
    LinkedListStack* Stack;								//����� ���� ���úκ�

    char Token[32];										//����� ���� ���� ������ ���ڿ�
    int  Type = -1;										//��Ģ������ ����
    unsigned int Position = 0;							//�Ի��� �� ������ �����ġ(������ ��尹���̹Ƿ� ����� ���)
    unsigned int Length = strlen( InfixExpression );	//����� �� �������� ����(�����̹Ƿ� ����� ���)
				
    LLS_CreateStack(&Stack);							//���� ����

    while ( Position < Length )							//����� �ϴ� ��ġ�� ���̺��� ���������ȸ�(���� ���� ���ȸ�) �ݺ����� ����
    {
        Position += GetNextToken( &InfixExpression[Position], Token, &Type );
														//�������� ���� ��ū�� �Ÿ���ŭ �̵�

        if ( Type == OPERAND )							//���� �ǿ������ϰ��
        {
            strcat( PostfixExpression, Token );			//��ū�� �����Ŀ� �̾������
            strcat( PostfixExpression, " " );			//����� ����
        }
        else if ( Type == RIGHT_PARENTHESIS )			//')'�� ���
        {               
            while ( !LLS_IsEmpty(Stack) )				//������ �������� �ݺ��� ����
            {
                Node* Popped = LLS_Pop( Stack );		//���� �� �����͸� ����

                if ( Popped->Data[0] == LEFT_PARENTHESIS )	//'('�� ���Դٸ�
                {
                    LLS_DestroyNode( Popped );			//�ش��带 ��ȯ
                    break;								//���� �ݺ����� ������ ��ȣ�� ����
                }
				else									//'('�� ������ �ʾҴٸ�
                {
                    strcat( PostfixExpression, Popped->Data );		//��ȣ���� ���ڵ��� �켱������ �����Ŀ� �̾����
                    LLS_DestroyNode( Popped );						//����� ��带 ��
                }
            }
        }
		else											//��Ģ������ ���
        {
            while ( !LLS_IsEmpty( Stack ) &&			//������ ������� �ʰ� 
                    !IsPrior( LLS_Top( Stack )->Data[0], Token[0] ) )	//������0�� �����ͺ��� ��ū��0�� �������� �켱���� ���� ��쿡�� �ݺ��� ����
            {
                Node* Popped = LLS_Pop( Stack );				//����� ��带 ����� �׳�忡 ������ ���� �Ǹ��������� �ְ� ����

                if ( Popped->Data[0] != LEFT_PARENTHESIS )		//ù��°�� '('�� �ƴҰ�쿡�� 
                    strcat( PostfixExpression, Popped->Data );	//����ǥ��Ŀ� ����� ����� �����͸� �̾���� 
                
                LLS_DestroyNode( Popped );						//����  �ش��带 ����
            }
            
            LLS_Push( Stack, LLS_CreateNode( Token ) );			//�ݺ����� ������ ���ÿ� ��ū�� ����ִ´�
        }
    }

    while ( !LLS_IsEmpty( Stack) )							//������ ���� ���������� �ݺ����� ������
    {
        Node* Popped = LLS_Pop( Stack );					//����� ��带 ����� �� ��忡  ������ ���� �Ǹ��������� �ְ� ����

        if ( Popped->Data[0] != LEFT_PARENTHESIS )			//���� ù��° �����Ͱ� '('�� �ƴҰ�쿡��
            strcat( PostfixExpression, Popped->Data );		//����ǥ��Ŀ� �����͸� �̾������
        
        LLS_DestroyNode( Popped );							//�׸��� ����� ��带 ����
    }

    LLS_DestroyStack(Stack);								//�ݺ����� ������ ������ ����
}

double Calculate( char* PostfixExpression )
{
    LinkedListStack* Stack;
    Node*  ResultNode;

    double Result;
    char Token[32];
    int  Type = -1;
    unsigned int Read = 0; 
    unsigned int Length = strlen( PostfixExpression );

    LLS_CreateStack(&Stack);
   
    while ( Read < Length )
    {
        Read += GetNextToken( &PostfixExpression[Read], Token, &Type );

        if ( Type == SPACE )  
            continue;
        
        if ( Type == OPERAND ) 
        {
            Node* NewNode = LLS_CreateNode( Token );
            LLS_Push( Stack, NewNode );
        }
        else
        {
            char   ResultString[32];            
            double Operator1, Operator2, TempResult;
            Node* OperatorNode;

            OperatorNode = LLS_Pop( Stack );
            Operator2 = atof( OperatorNode->Data );
            LLS_DestroyNode( OperatorNode );

            OperatorNode = LLS_Pop( Stack );
            Operator1 = atof( OperatorNode->Data );
            LLS_DestroyNode( OperatorNode );
            
            switch (Type)
            {
            case PLUS:     TempResult = Operator1 + Operator2; break;
            case MINUS:    TempResult = Operator1 - Operator2; break;
            case MULTIPLY: TempResult = Operator1 * Operator2; break;
            case DIVIDE:   TempResult = Operator1 / Operator2; break;
            }

            gcvt( TempResult, 10, ResultString );            
            LLS_Push( Stack, LLS_CreateNode( ResultString ) );
        }
    }

    ResultNode = LLS_Pop( Stack );            
    Result = atof( ResultNode->Data );
    LLS_DestroyNode( ResultNode );

    LLS_DestroyStack( Stack );

    return Result;
}
