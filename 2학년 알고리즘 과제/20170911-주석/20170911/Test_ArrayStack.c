#include "ArrayStack.h"

int main( void )
{		
    ArrayStack* Stack = NULL;			//���� ����

    AS_CreateStack(&Stack, 10);			//�ѿ뷮�� 10���� ����
    
    AS_Push(Stack, 3);					//ù��° �������Է� ���� ž 3
    AS_Push(Stack, 37);					//�ι�° �������Է� ���� ž 37
    AS_Push(Stack, 11);					//����° �������Է� ���� ž 11
    AS_Push(Stack, 12);					//�׹�° �������Է� ���� ž 12

    printf( "Capacity: %d, Size: %d, Top: %d\n\n",					//���
        Stack->Capacity, AS_GetSize(Stack), AS_Top(Stack) );		//������ �뷮,������ ũ��,���� ������ ž

	for (int i = 0; i < 4; i++)			
	{
		if (AS_IsEmpty(Stack))			//���ÿ� �����Ͱ� ������� �������
			break;						//�ݺ��� ����

		printf("Popped: %d, ", AS_Pop(Stack));				//�� ���� �����͸� �˾Ƴ��� ����� ��

		if (!AS_IsEmpty(Stack))								//���� ������ ��ĭ�� �ƴҰ��
		{
			printf("Current Top: %d\n", AS_Top(Stack));		//���� ž�� ������ ���
		}

		else
		{
			printf("Stack Is Empty.\n");					//������ ��ĭ�̸� �޼��� ���
		}
	}

    AS_DestroyStack(Stack);									//���� ��ȯ

    return 0;
}