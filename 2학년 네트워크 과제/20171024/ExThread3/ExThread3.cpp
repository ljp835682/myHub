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
		(LPVOID)num, CREATE_SUSPENDED, NULL);				//CREATE_SUSPENDED로 설정하면 만들지만 아직 CPU의 시간을 할당 받지 않은 스레드함수를 만듬
	if(hThread == NULL) return 1;

	printf("스레드 실행 전. 계산 결과 = %d\n", sum);
	ResumeThread(hThread);									//멈춰있던 스레드를 CPU시간을 할당 받을수 있게해주는 함수
	WaitForSingleObject(hThread, INFINITE);					//hThread 스레드가 끝나는걸 기다린다, INFINITE부분은 밀리초세컨드 단위로 하면 그만큼만 기다린다, 근데 INFINITE 를 사용하면 끝날때까지 계속 기다림
															//WaitForMultipleObjects 함수 : (기다릴 갯수,스레드의 배열의 이름,전부다 기다리면 true 아니면 false,기다릴 시간)
															//	return값 : bWaitAll이 false의 경우 끝났던 배열의 멤버의 인덱스 번호를 리턴함
	
	printf("스레드 실행 후. 계산 결과 = %d\n", sum);		
	CloseHandle(hThread);
	
	return 0;
}