#include <windows.h>
#include <stdio.h>

int sum = 0;

DWORD WINAPI MyThread(LPVOID arg)
{
	int num = (int)arg;
	for(int i=1; i<=num; i++) sum += i;
	return 0;
}

int main(int argc, char *argv[])
{
	int num = 100;
	HANDLE hThread = CreateThread(NULL, 0, MyThread, 
		(LPVOID)num, CREATE_SUSPENDED, NULL);				//CREATE_SUSPENDED�� �����ϸ� �������� ���� CPU�� �ð��� �Ҵ� ���� ���� �������Լ��� ����
	if(hThread == NULL) return 1;

	printf("������ ���� ��. ��� ��� = %d\n", sum);
	ResumeThread(hThread);									//�����ִ� �����带 CPU�ð��� �Ҵ� ������ �ְ����ִ� �Լ�
	WaitForSingleObject(hThread, INFINITE);					//hThread �����尡 �����°� ��ٸ���, INFINITE�κ��� �и��ʼ����� ������ �ϸ� �׸�ŭ�� ��ٸ���, �ٵ� INFINITE �� ����ϸ� ���������� ��� ��ٸ�
															//WaitForMultipleObjects �Լ� : (��ٸ� ����,�������� �迭�� �̸�,���δ� ��ٸ��� true �ƴϸ� false,��ٸ� �ð�)
															//	return�� : bWaitAll�� false�� ��� ������ �迭�� ����� �ε��� ��ȣ�� ������
	
	printf("������ ���� ��. ��� ��� = %d\n", sum);		
	CloseHandle(hThread);
	
	return 0;
}