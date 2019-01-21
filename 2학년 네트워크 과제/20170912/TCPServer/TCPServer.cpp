#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

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
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
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
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
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
	srand((unsigned int)time(NULL));

	int retval;

	// 윈속 초기화
	WSADATA wsa;									
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)		
		return 1;									

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);		
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");		

	// bind()
	SOCKADDR_IN serveraddr;									
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;				
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);	
	serveraddr.sin_port = htons(SERVERPORT);		
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");

	// listen()											
	retval = listen(listen_sock, SOMAXCONN);			
	if(retval == SOCKET_ERROR) err_quit("listen()");	

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;					
	char buf[BUFSIZE+1];
	int Count = 0;
	char * Intro = "1부터 10까지 수 입력 : ";

	while(1)
	{
		Count = 0;		//클라연결전에 초기화
		// accept()										
		addrlen = sizeof(clientaddr);				
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);		
		if(client_sock == INVALID_SOCKET)											
		{
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		int Number = rand() % 10 + 1;			//랜덤 생성

		retval = send(client_sock, Intro, strlen(Intro), 0);					//문자열을 보내야하므로 size of가 아닌 strlen을 보냄
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}

		// 클라이언트와 데이터 통신
		while(1)
		{
			// 데이터 받기	
			int Value = 0;														//클라에게 받을 숫자
			retval = recvn(client_sock, (char*)&Value, sizeof(Value), 0);		//문자열을 받기위해 char형이 아니라 바이트단위로 받기위해 char인것

			if(retval == SOCKET_ERROR)												
			{
				err_display("recv()");
				break;
			}

			else if (retval == 0)
			{
				break;
			}

			Count++;															//입력받았으므로 횟수증가
			int Result;
			
			if (Value > Number)
			{
				Result = DOWN;
			}

			else if (Value < Number)											//입력받은수와 정답 비교
			{
				Result = UP;
			}

			else
			{
				Result = OK;
			}								

			// 데이터 보내기
			retval = send(client_sock, (char*)&Result, sizeof(Result), 0);

			if(retval == SOCKET_ERROR)
			{
				err_display("send()");
				break;
			}

			if (Result == OK)					//만약 정답일경우
			{
				break;							//반복문 탈출
			}
		}

		sprintf(buf, "%d번만에 맞추셨습니다.", Count);			//보낼 문자열 생성
		retval = send(client_sock, buf, strlen(buf), 0);		//클라에게 보냄

		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}

		// closesocket()
		closesocket(client_sock);													
																					
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",			
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket(listen_sock);														

	// 윈속 종료
	WSACleanup();															
	return 0;
}