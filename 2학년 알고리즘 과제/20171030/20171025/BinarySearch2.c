#include <stdlib.h> 
#include <stdio.h>
#include "Score.h"

int CompareScore( const void *_elem1, const void *_elem2 )			//기준정렬방법
{ 
    Score* elem1 = (Score*)_elem1;									
    Score* elem2 = (Score*)_elem2;									//인자로 온 비교할 기준포인터를 Score 포인터 형으로 변환
 
    if ( elem1->score > elem2->score )								//1 이 크다면 1을 리턴
        return 1; 
    else if ( elem1->score < elem2->score )							//2 가 크다면 -1을 리턴
        return -1; 
    else															//같다면 0을 리턴
        return 0;     
} 

int main( void )
{
    int Length = sizeof DataSet / sizeof DataSet[0];						//비교할 배열의 크기
    int i = 0;																//반복을 돌리는데 사용할 변수
    Score  target;															//찾을값을 저장할 변수
    Score* found  = NULL;													//찾아낸 값을 저장할 포인터 변수
	float input = 0;

    /*  점수의 오름차순으로 정렬 */
    qsort( (void*)DataSet, Length, sizeof (Score), CompareScore );			//바이너리 서치를 실행하기전에 퀵 소트를 사용하여 정렬

    /*  671.78 점을 받은 학생 찾기 */
    target.number = 0;														//number는 사용하지않음
    //target.score  = 671.78;												//671.78점을 찾아야하므로 지정

	scanf("%lf", &target.score);											//입력

    found = bsearch(														//바이너리 서치함수 실행
             (void*)&target,												//찾아야하는 타겟
             (void*)DataSet,												//찾아야할 공간
             Length,														//찾아야할 공간의 길이
             sizeof (Score),												//찾아야할 자료형
             CompareScore );												//기준정렬방법 함수 포인터
    
    printf("found: %d %lf \n", found->number, found->score );				//찾아낸값 출력
 
    return 0;
}