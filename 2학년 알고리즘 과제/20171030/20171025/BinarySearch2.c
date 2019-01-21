#include <stdlib.h> 
#include <stdio.h>
#include "Score.h"

int CompareScore( const void *_elem1, const void *_elem2 )			//�������Ĺ��
{ 
    Score* elem1 = (Score*)_elem1;									
    Score* elem2 = (Score*)_elem2;									//���ڷ� �� ���� ���������͸� Score ������ ������ ��ȯ
 
    if ( elem1->score > elem2->score )								//1 �� ũ�ٸ� 1�� ����
        return 1; 
    else if ( elem1->score < elem2->score )							//2 �� ũ�ٸ� -1�� ����
        return -1; 
    else															//���ٸ� 0�� ����
        return 0;     
} 

int main( void )
{
    int Length = sizeof DataSet / sizeof DataSet[0];						//���� �迭�� ũ��
    int i = 0;																//�ݺ��� �����µ� ����� ����
    Score  target;															//ã������ ������ ����
    Score* found  = NULL;													//ã�Ƴ� ���� ������ ������ ����
	float input = 0;

    /*  ������ ������������ ���� */
    qsort( (void*)DataSet, Length, sizeof (Score), CompareScore );			//���̳ʸ� ��ġ�� �����ϱ����� �� ��Ʈ�� ����Ͽ� ����

    /*  671.78 ���� ���� �л� ã�� */
    target.number = 0;														//number�� �����������
    //target.score  = 671.78;												//671.78���� ã�ƾ��ϹǷ� ����

	scanf("%lf", &target.score);											//�Է�

    found = bsearch(														//���̳ʸ� ��ġ�Լ� ����
             (void*)&target,												//ã�ƾ��ϴ� Ÿ��
             (void*)DataSet,												//ã�ƾ��� ����
             Length,														//ã�ƾ��� ������ ����
             sizeof (Score),												//ã�ƾ��� �ڷ���
             CompareScore );												//�������Ĺ�� �Լ� ������
    
    printf("found: %d %lf \n", found->number, found->score );				//ã�Ƴ��� ���
 
    return 0;
}