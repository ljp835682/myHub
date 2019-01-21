#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"		
#define SERVERPORT 9000			
#define BUFSIZE    512
#define STRSIZE    128

enum RESULT{ NODATA = 0, WIN, LOOSE, DRAW };
enum PROTOCOL{ INTRO, INPUTDATA, GAME_RESULT };

struct User_Info		//유저의 정보를 저장할 구조체
{
	char * Result;
	int HandState;
};


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
		return -1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	char buf[BUFSIZE];
	int size;


	while (1)
	{
		retval = recvn(sock, (char*)&size, sizeof(int), 0);		//전체 받을 사이즈 알아봄
		if (retval == SOCKET_ERROR)
		{
			err_quit("recv()");
		}
		else if (retval == 0)
		{
			return 0;
		}

		retval = recvn(sock, (char*)buf, size, 0);				//알아낸 사이즈만큼 받음
		if (retval == SOCKET_ERROR)
		{
			err_quit("recv()");
		}
		else if (retval == 0)
			return 0;

		PROTOCOL protocol;
		char* ptr = buf;										

		memcpy(&protocol, ptr, sizeof(PROTOCOL));				//프로토콜
		ptr = ptr + sizeof(PROTOCOL);							//프로토콜의 크기만큼 이동

		// 데이터 통신에 사용할 변수
		bool endflag = false;

		switch (protocol)
		{
		case INTRO:												//인트로의 경우

			int intro_size;
			memcpy(&intro_size, ptr, sizeof(int));				//인트로의 문자열 길이
			ptr = ptr + sizeof(intro_size);						

			char intro[BUFSIZE];
			memcpy(intro, ptr, intro_size);						//인트로 문자열
			ptr = ptr + intro_size;

			intro[intro_size] = '\0';							//끝에 널문자
			printf("%s\n", intro);								//출력
			break;

		case GAME_RESULT:

			endflag = true;										//반복문 탈출용 플래그

			int game_result_size;
			memcpy(&game_result_size, ptr, sizeof(int));		//결과 문자열 길이
			ptr = ptr + sizeof(int);

			char game_result[BUFSIZE];
			memcpy(game_result, ptr, game_result_size);			//결과 문자열
			ptr = ptr + game_result_size;

			game_result[game_result_size] = '\0';				//끝에 널문자
			printf("%s\n", game_result);						//출력
			break;
		}

		if (endflag)
		{
			break;												//GAME_RESULT 였을경우 종료
		}

		User_Info info;
		ZeroMemory(&info, sizeof(info));
		printf("입력 :");										//클라의 입력
		scanf("%d",&info.HandState);							

		ptr = buf;
		protocol = INPUTDATA;									
		size = sizeof(size)+sizeof(protocol)+sizeof(info.HandState);

		memcpy(ptr, &size, sizeof(size));
		ptr = ptr + sizeof(size);

		memcpy(ptr, &protocol, sizeof(protocol));
		ptr = ptr + sizeof(protocol);

		memcpy(ptr, &info.HandState, sizeof(info.HandState));
		ptr = ptr + sizeof(info.HandState);								//전체 사이즈 + 프로토콜 + 유저의 선택

		retval = send(sock, buf, sizeof(int)+size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}

	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
