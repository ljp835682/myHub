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

void GetPostfix( char* InfixExpression, char* PostfixExpression )		//중위식,후위식
{
    LinkedListStack* Stack;								//계산을 위한 스택부분

    char Token[32];										//계산을 위한 식을 저장할 문자열
    int  Type = -1;										//사칙연산의 종류
    unsigned int Position = 0;							//게산을 할 스택의 노드위치(스택의 노드갯수이므로 양수만 취급)
    unsigned int Length = strlen( InfixExpression );	//계산을 할 중위식의 길이(길이이므로 양수만 취급)
				
    LLS_CreateStack(&Stack);							//스택 생성

    while ( Position < Length )							//계산을 하는 위치가 길이보다 작을때동안만(식을 읽을 동안만) 반복문을 실행
    {
        Position += GetNextToken( &InfixExpression[Position], Token, &Type );
														//포지션을 다음 토큰의 거리만큼 이동

        if ( Type == OPERAND )							//만약 피연산자일경우
        {
            strcat( PostfixExpression, Token );			//토큰을 후위식에 이어붙히고
            strcat( PostfixExpression, " " );			//띄어쓰기로 구분
        }
        else if ( Type == RIGHT_PARENTHESIS )			//')'의 경우
        {               
            while ( !LLS_IsEmpty(Stack) )				//스택이 빌때까지 반복문 실행
            {
                Node* Popped = LLS_Pop( Stack );		//식의 옆 데이터를 빼고

                if ( Popped->Data[0] == LEFT_PARENTHESIS )	//'('가 나왔다면
                {
                    LLS_DestroyNode( Popped );			//해당노드를 반환
                    break;								//그후 반복문을 나가서 괄호를 끝냄
                }
				else									//'('가 나오지 않았다면
                {
                    strcat( PostfixExpression, Popped->Data );		//괄호안의 숫자들을 우선적으로 후위식에 이어붙힘
                    LLS_DestroyNode( Popped );						//사용한 노드를 뺌
                }
            }
        }
		else											//사칙연산의 경우
        {
            while ( !LLS_IsEmpty( Stack ) &&			//스택이 비어있지 않고 
                    !IsPrior( LLS_Top( Stack )->Data[0], Token[0] ) )	//스택의0번 데이터보다 토큰의0번 데이터의 우선권이 높을 경우에만 반복문 실행
            {
                Node* Popped = LLS_Pop( Stack );				//저장용 노드를 만들고 그노드에 스택의 가장 맨마지막값을 넣고 뺀다

                if ( Popped->Data[0] != LEFT_PARENTHESIS )		//첫번째가 '('이 아닐경우에만 
                    strcat( PostfixExpression, Popped->Data );	//후위표기식에 저장용 노드의 데이터를 이어붙힘 
                
                LLS_DestroyNode( Popped );						//그후  해당노드를 지움
            }
            
            LLS_Push( Stack, LLS_CreateNode( Token ) );			//반복문이 끝나면 스택에 토큰을 집어넣는다
        }
    }

    while ( !LLS_IsEmpty( Stack) )							//스택이 비지 않을때까지 반복문을 돌린다
    {
        Node* Popped = LLS_Pop( Stack );					//저장용 노드를 만들고 그 노드에  스택의 가장 맨마지막값을 넣고 뺀다

        if ( Popped->Data[0] != LEFT_PARENTHESIS )			//만약 첫번째 데이터가 '('가 아닐경우에만
            strcat( PostfixExpression, Popped->Data );		//후위표기식에 데이터를 이어붙힌다
        
        LLS_DestroyNode( Popped );							//그리고 저장용 노드를 삭제
    }

    LLS_DestroyStack(Stack);								//반복문이 끝나면 스택을 삭제
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
