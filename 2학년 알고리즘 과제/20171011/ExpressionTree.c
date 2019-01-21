#include "ExpressionTree.h"

ETNode* ET_CreateNode( ElementType NewData )				//초기화
{
    ETNode* NewNode = (ETNode*)malloc( sizeof(ETNode) );
    NewNode->Left    = NULL;
    NewNode->Right   = NULL;
    NewNode->Data    = NewData;

    return NewNode;
}

void ET_DestroyNode( ETNode* Node )							//노드의 삭제
{
    free(Node);
}

void ET_DestroyTree( ETNode* Root )							//트리의 삭제
{
    if ( Root == NULL )										//만약 자신이 이진트리가 아니라면
        return;												//그냥 종료

    ET_DestroyTree( Root->Left );							//재귀함수로 왼쪽의 노드가 이진트리라면 삭제
    ET_DestroyTree( Root->Right );							//재귀함수로 오른쪽의 노드가 이진트리라면 삭제
    ET_DestroyNode( Root );									//자신을 삭제
}

void ET_PreorderPrintTree( ETNode* Node )					//전위식 출력
{
    if ( Node == NULL )
        return;												//만약 빈 노드면 종료

    printf( " %c", Node->Data );							//첫번째로 자신의 데이터를 먼저 출력

    ET_PreorderPrintTree( Node->Left );						//두번째로 왼쪽 노드의 데이터를 출력
    ET_PreorderPrintTree( Node->Right );					//세번째로 오른쪽 노드의 데이터를 출력
}															//결과적으로 계산할 토큰이 앞에오고 그 오른쪽 두개에 계산할 수가 오게될것  

void ET_InorderPrintTree( ETNode* Node )					//중위식 출력
{
    if ( Node == NULL )
        return;												//만약 빈 노드면 종료
    
    printf( "(" );
    ET_InorderPrintTree( Node->Left );						//첫번째로 왼쪽의 데이터를 재귀함수로 출력

    printf( "%c", Node->Data );								//두번째로 자신의 데이터를 출력

    ET_InorderPrintTree( Node->Right );						//세번째로 오른쪽의 데이터를 재귀함수로 출력
    printf( ")" );				
}															//결과적으로 가운데에 토큰이 오고 왼쪽과 오른쪽에 계산할 수가 오게될것

void ET_PostorderPrintTree( ETNode* Node )					//후위식 출력
{
    if ( Node == NULL )										//만약 빈 노드면 종료
        return;
    
    ET_PostorderPrintTree( Node->Left );					//첫번째로 왼쪽의 노드의 데이터를 먼저 출력
    ET_PostorderPrintTree( Node->Right );					//두번째로 오른쪽의 노드의 데이터를 출력
    printf( " %c", Node->Data );							//세번째로 자신의 데이터를 출력
}

void ET_BuildExpressionTree( char* PostfixExpression, ETNode** Node )		//계산식트리 생성 함수
{
    ETNode* NewNode = NULL;
    int  len        = strlen( PostfixExpression );			//식의 길이
    char Token      = PostfixExpression[ len -1 ];			//널문자가 아닌 마지막 문자
    PostfixExpression[ len-1 ] = '\0';						//토큰으로 빼낸 문자의 위치에 널문자 대입

    switch ( Token ) 
    {
        /*  연산자인 경우 */
        case '+': case '-': case '*': case '/':				
            (*Node) = ET_CreateNode( Token );			//토큰은 자식이 생겨야함
            ET_BuildExpressionTree( PostfixExpression, &(*Node)->Right );	//첫번째로 오른쪽에 다음 토큰을 자식으로 
            ET_BuildExpressionTree( PostfixExpression, &(*Node)->Left  );	//두번째로 왼쪽에 다음 토큰을 자식으로
            break;

        /*  피연산자인 경우 */
        default :
            (*Node) = ET_CreateNode( Token);			//숫자의 경우 자식이 생길일이 없으므로 그냥 노드생성
            break;
    }
}

double ET_Evaluate( ETNode* Tree )						//트리 계산 함수
{
    char Temp[2];
    
    double Left   = 0;									//왼쪽의 트리에서 계산될값
    double Right  = 0;									//오른쪽 트리에서 계산될값
    double Result = 0;									//결과값

    if ( Tree == NULL )									//재귀함수의 끝을 정하기위해 널일경우 함수종료
        return 0;

    switch ( Tree->Data ) 
    {
        /*  연산자인 경우 */
        case '+': case '-': case '*': case '/':			//계산을 해야함
            Left  = ET_Evaluate( Tree->Left );			//왼쪽 트리가 있다면 먼저 계산하고
            Right = ET_Evaluate( Tree->Right );			//오른쪽 트리가 있다면 계산하여 각각 저장

                 if ( Tree->Data == '+' ) Result = Left + Right;
            else if ( Tree->Data == '-' ) Result = Left - Right;
            else if ( Tree->Data == '*' ) Result = Left * Right;
            else if ( Tree->Data == '/' ) Result = Left / Right;		//맞는 계산을 실행

            break;

        /*  피연산자인 경우 */
        default :
            memset(Temp, 0, sizeof(Temp));		//빈칸으로 바꿔두고
            Temp[0] = Tree->Data;				//데이터를 넣어서
            Result = atof(Temp);				//문자를 실수로
            break;
    }

    return Result;				//결과를 리턴
}