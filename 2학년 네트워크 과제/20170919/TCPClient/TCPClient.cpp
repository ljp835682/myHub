#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"		
#define SERVERPORT 9000			
#define BUFSIZE    512

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

	// 서버와 데이터 통신
	while (1)
	{
		// recv() 어떤 문자열이 올지 모르기에 recv를 사용함
		retval = recv(sock, buf, BUFSIZE, 0);		
		
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			{
				break;
			}
		}

		else if (retval == 0)
		{
			break;
		}

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("%s\n", buf);

		int Choice;

		//오류방지용 반복문
		while (1)
		{
			printf("0. 가위, 1. 바위, 2. 보... :  ");
			scanf("%d", &Choice);

			if (Choice >= 0 && Choice <= 2)
			{
				break;
			}
		}

		// send()서버에게 자신이 고른것을 보냄
		retval = send(sock, (char *)&Choice, sizeof(Choice), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			{
				break;
			}
		}

		// recv() 서버에게서 문자열을 받음
		retval = recv(sock, buf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			{
				break;
			}
		}

		//받은문자열 출력
		buf[retval] = '\0';
		printf("%s\n", buf);

		break;
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
