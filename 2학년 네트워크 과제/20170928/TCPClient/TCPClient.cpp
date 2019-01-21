#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"		
#define SERVERPORT 9000			
#define BUFSIZE    512
#define STRSIZE    128

enum RESULT{ NODATA = 0, IDERROR, PWERROR, LOGINSUCESS };
enum PROTOCOL{ INTRO, ID_PW_INFO, LOGIN_RESULT };

struct User_Info
{
	char ID[10];
	char PW[10];
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
		retval = recvn(sock, (char*)&size, sizeof(int), 0);
		if (retval == SOCKET_ERROR)
		{
			err_quit("recv()");
		}
		else if (retval == 0)
		{
			return 0;
		}

		retval = recvn(sock, (char*)buf, size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_quit("recv()");
		}
		else if (retval == 0)
			return 0;

		PROTOCOL protocol;
		char* ptr = buf;

		memcpy(&protocol, ptr, sizeof(PROTOCOL));
		ptr = ptr + sizeof(PROTOCOL);

		// 데이터 통신에 사용할 변수
		bool endflag = false;

		switch (protocol)
		{
		case INTRO:

			int intro_size;
			memcpy(&intro_size, ptr, sizeof(int));
			ptr = ptr + sizeof(int);

			char intro[BUFSIZE];
			memcpy(intro, ptr, intro_size);
			ptr = ptr + intro_size;

			intro[intro_size] = '\0';
			printf("%s\n", intro);

			break;

		case LOGIN_RESULT:

			int result;
			memcpy(&result, ptr, sizeof(int));
			ptr = ptr + sizeof(int);

			int result_size;
			memcpy(&result_size, ptr, sizeof(int));
			ptr = ptr + sizeof(int);

			char result_tmp[BUFSIZE];
			memcpy(result_tmp, ptr, result_size);

			result_tmp[result_size] = '\0';
			printf("%s\n", result_tmp);

			if (result == LOGINSUCESS)
			{
				endflag = true;
			}

			break;
		}

		if (endflag)
		{
			break;
		}

		User_Info info;
		ZeroMemory(&info, sizeof(info));
		printf("ID:");
		gets(info.ID);
		printf("PW:");
		gets(info.PW);

		ptr = buf;
		protocol = ID_PW_INFO;
		int idsize = strlen(info.ID);
		int pwsize = strlen(info.PW);
		size = sizeof(int)+sizeof(PROTOCOL)+sizeof(int)+idsize + sizeof(int)+pwsize;

		memcpy(ptr, &size, sizeof(int));
		ptr = ptr + sizeof(int);

		memcpy(ptr, &protocol, sizeof(PROTOCOL));
		ptr = ptr + sizeof(PROTOCOL);

		memcpy(ptr, &idsize, sizeof(int));
		ptr = ptr + sizeof(int);

		memcpy(ptr, info.ID, idsize);
		ptr = ptr + idsize;

		memcpy(ptr, &pwsize, sizeof(int));
		ptr = ptr + sizeof(int);

		memcpy(ptr, info.PW, pwsize);

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
