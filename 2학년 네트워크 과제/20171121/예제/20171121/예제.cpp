#include <windows.h>
#include <stdio.h>

int number = 0;

HANDLE PlusEvent;
HANDLE MinusEvent;

DWORD WINAPI PlusTread(LPVOID arg)									//����������
{
	for(int i=0;i<100;i++)
	{
		int retval = WaitForSingleObject(MinusEvent, INFINITE);		//�����̺�Ʈ�� ��ȣ���·� �Ǵ°� ��ٸ��ϴ� �����̺�Ʈ��
																	//��ȣ���·� �����Ǿ����Ƿ� ó������ �׳� �������ϴ�
		number+=3;													//3����

		printf("3���� : number : %d\n", number);					

		SetEvent(PlusEvent);										//�����̺�Ʈ�� ��ȣ���·� �����մϴ�
	}																

	return 0;
}

DWORD WINAPI MinusTread(LPVOID arg)
{
	for (int i = 0; i<100; i++)
	{
		int retval = WaitForSingleObject(PlusEvent, INFINITE);		//���������尡 ��ȣ���·� �Ǵ°� ��ٸ��ϴ�
		number-=2;													//2����

		printf("2���� : number : %d\n", number);

		SetEvent(MinusEvent);										//���ҽ����带 ��ȣ���·� �����մϴ�
	}

	return 0;
}

int main()
{
	PlusEvent = CreateEvent(NULL, FALSE, FALSE, NULL);				//���� �̺�Ʈ�� ���ȣ���·� ����
	if (PlusEvent == NULL) return 1;
	MinusEvent = CreateEvent(NULL, FALSE, TRUE, NULL);				//���� �̺�Ʈ�� ��ȣ ���·� ����
	if (MinusEvent == NULL) return 1;

	printf("number : %d\n", number);								//�ʱⰪ ���

	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, PlusTread, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, MinusTread, NULL, 0, NULL);	//�ΰ��� ������ ����

	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);				//�ΰ��� �����ϱ����� ���ν����尡 ����Ǵ°� ����

	CloseHandle(PlusEvent);
	CloseHandle(MinusEvent);										//�̺�Ʈ ����

	return 0;									
}