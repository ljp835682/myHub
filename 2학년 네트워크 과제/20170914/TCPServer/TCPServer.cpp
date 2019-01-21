#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "User.h"

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

/*
과제 err_display 가 어떤형식으로 돌아가는지 조사해올것
*/

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;		//보이드 포인터
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

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
	cUser * User = new cUser[3];
	User[0].SetIDPW("AAA", "AAA");
	User[1].SetIDPW("BBB", "BBB");
	User[2].SetIDPW("CCC", "CCC");

	char * Intro = "connecting success.";

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()											
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	while (1)
	{
		// accept()										
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
		{
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 인트로 송신
		send(client_sock, Intro, strlen(Intro), 0);

		// 클라이언트와 데이터 통신
		while (1)
		{			
			//유저 객체 수신
			cUser * Temp = new cUser();
			retval = recv(client_sock, (char*)Temp, BUFSIZE, 0);	//객체속 문자열 크기 알수없음 recv 사용

			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}

			else if (retval == 0)
			{
				break;
			}

			// 받은 객체와 가진 데이터 비교
			int Msgflag = 0;

			for (int i = 0; i < 3; i++)
			{
				int Answer = User[i].LoginCheck(Temp->GetID(), Temp->GetPW());

				if (Answer == OK)
				{
					Msgflag = OK;
					break;
				}

				else if (Answer == MISS)
				{
					Msgflag = MISS;
					break;
				}

				else
				{
					Msgflag = NO;
				}
			}

			//클라이언트의 입력 정보 삭제
			delete Temp;

			//결과 송신
			send(client_sock, (char *)&Msgflag, sizeof(Msgflag), 0);

			//로그인성공시 클라이언트 접속 종료
			if (Msgflag == OK)
			{
				break;
			}
		}

		// closesocket()
		closesocket(client_sock);

		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}
	//유저정보 삭제
	delete User;

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

/*

로그인서버 만들기
클라가 서버에게 붙으면 아이디와 패스워드 입력
서버가 이미 몇개 아이디와 비밀번호를 가지고있고
아이디가없으면 아이디가 없다고 알려주고
패스워드가 다르면 패스워드가 다르다고 달려주고
로그인되면 로그인성공
로그인이 실패할경우 다시 계속 로그인시도
회원의 정보는 구조체로 생성해서 한번에 보냄
똑같은 형태의 구조체를 클라와 서버 둘다 가지고잇어야함

*/