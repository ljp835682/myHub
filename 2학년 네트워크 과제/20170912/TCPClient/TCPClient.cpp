#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"		
#define SERVERPORT 9000			
#define BUFSIZE    512
#define UP	3
#define OK 2
#define DOWN 1

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0)
	{
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)		
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;					
		ptr += received;				
	}

	return (len - left);					
}											

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);	
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);				
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));	
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE + 1];
	int len;

	retval = recv(sock, buf, strlen(buf), 0);		//인트로 문자열 받음
	buf[retval] = '\0';								//맨끝에 널문자
	
	int InputNum = 0;
	printf("%s", buf);				//받은 문자열 출력


	// 서버와 데이터 통신
	while (1)
	{
		scanf("%d", &InputNum);		//숫자입력받음
		printf("\n");

		// 숫자 데이터 전송
		retval = send(sock, (char *)&InputNum, sizeof(InputNum), 0);		//서버에게 숫자를 보냄
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}

		//결과 받음
		int Result;					//결과 저정할 공간
		retval = recvn(sock, (char *)&Result, sizeof(Result), 0);			//Result에 저장할 값을 받음
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}

		bool Outflag = false;		//반복문 탈출용 변수

		switch (Result)
		{
		case UP:					//업일경우
			printf("업!\n");		
			break;

		case DOWN:					//다운일경우
			printf("다운!\n");
			break;
				
		case OK:					//정답일경우
			printf("정답!\n");
			
			retval = recv(sock, buf, strlen(buf), 0);		//몇번만에 맞췄는지 문자열받음
			buf[retval] = '\0';								//마지막에 널문자

			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}

			printf("%s\n", buf);							//받은 문자열 출력

			Outflag = true;
			break;
		}

		if (Outflag)
		{
			break;			//반복문 탈출
		}
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}