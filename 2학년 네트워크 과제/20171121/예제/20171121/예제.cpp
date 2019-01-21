#include <windows.h>
#include <stdio.h>

int number = 0;

HANDLE PlusEvent;
HANDLE MinusEvent;

DWORD WINAPI PlusTread(LPVOID arg)									//증가스레드
{
	for(int i=0;i<100;i++)
	{
		int retval = WaitForSingleObject(MinusEvent, INFINITE);		//감소이벤트가 신호상태로 되는걸 기다립니다 감소이벤트는
																	//신호상태로 생성되었으므로 처음에는 그냥 지나갑니다
		number+=3;													//3증가

		printf("3증가 : number : %d\n", number);					

		SetEvent(PlusEvent);										//증가이벤트를 신호상태로 변경합니다
	}																

	return 0;
}

DWORD WINAPI MinusTread(LPVOID arg)
{
	for (int i = 0; i<100; i++)
	{
		int retval = WaitForSingleObject(PlusEvent, INFINITE);		//증가스레드가 신호상태로 되는걸 기다립니다
		number-=2;													//2감소

		printf("2감소 : number : %d\n", number);

		SetEvent(MinusEvent);										//감소스레드를 신호상태로 변경합니다
	}

	return 0;
}

int main()
{
	PlusEvent = CreateEvent(NULL, FALSE, FALSE, NULL);				//증가 이벤트를 비신호상태로 생성
	if (PlusEvent == NULL) return 1;
	MinusEvent = CreateEvent(NULL, FALSE, TRUE, NULL);				//감소 이벤트를 신호 상태로 생성
	if (MinusEvent == NULL) return 1;

	printf("number : %d\n", number);								//초기값 출력

	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, PlusTread, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, MinusTread, NULL, 0, NULL);	//두개의 스레드 실행

	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);				//두개가 종료하기전에 메인스레드가 종료되는걸 방지

	CloseHandle(PlusEvent);
	CloseHandle(MinusEvent);										//이벤트 삭제

	return 0;									
}