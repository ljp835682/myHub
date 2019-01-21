#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "User.h"

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
	char ID[BUFSIZE];
	char PW[BUFSIZE];
	cUser * User = new cUser;
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
	char buf[BUFSIZE + 1] = " ";
	int len;

	// 인트로 수신
	recv(sock, buf, BUFSIZE, 0);	//문자열 받을 데이터의 크기 알수없음 recv사용		
	buf[strlen(buf) + 1] = '\0';				
	printf("%s\n", buf);

	// 서버와 데이터 통신
	while (1) {

		//아이디입력
		printf("ID : ");
		scanf("%s", ID);
		printf("PW : ");
		scanf("%s", PW);

		User->SetIDPW(ID, PW);

		// 유저 객체 송신
		retval = send(sock, (char*)User, sizeof(cUser), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}

		else if (retval == 0)
		{
			break;
		}

		//결과 수신
		int Msgflag = 0;
		retval = recvn(sock, (char *)&Msgflag, sizeof(Msgflag), 0);		//미리 약속해둔 심볼 받을 데이터의 크기 알수잇음 recvn사용
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}

		//결과값 비교
		char Message[BUFSIZE];

		switch (Msgflag)
		{
		case OK:
			strcpy_s(Message, strlen("success") + 1, "success");
			break;
		case MISS:
			strcpy_s(Message, strlen("mismatch") + 1, "mismatch");
			break;
		case NO:
			strcpy_s(Message, strlen("fail") + 1, "fail");
			break;
		}

		buf[retval] = '\0';

		//메세지 출력
		printf("%s\n", Message);

		//종료전 처리
		if (Msgflag == OK)
		{
			delete User;
			break;
		}
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
