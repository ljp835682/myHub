#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"		
#define SERVERPORT 9000			
#define BUFSIZE    512

enum PROTOCOL { INTRO, LOGIN, RESULT };

void PackPacket(char * packetbuf, PROTOCOL protocol, char * str1, char * str2, int & size)
{
	char * ptr = packetbuf;
	int strsize1 = strlen(str1);
	int strsize2 = strlen(str2);
	size = sizeof(protocol) + sizeof(strsize1) + strsize1 + sizeof(strsize2) + strsize2;

	memcpy(ptr, &size, sizeof(size));
	ptr += sizeof(size);

	memcpy(ptr, &protocol, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	memcpy(ptr, &strsize1, sizeof(strsize1));
	ptr = ptr + sizeof(strsize1);

	memcpy(ptr, str1, strsize1);
	ptr = ptr + strsize1;

	memcpy(ptr, &strsize2, sizeof(strsize2));
	ptr = ptr + sizeof(strsize2);

	memcpy(ptr, str2, strsize2);
	ptr = ptr + strsize2;

	size = size + sizeof(size);
}

void GetProtocol(char * packetbuf, PROTOCOL & protocol)
{
	memcpy(&protocol, packetbuf, sizeof(PROTOCOL));
}

void UnPackPacket(char * packetbuf, char * str1)
{
	char * ptr = packetbuf + sizeof(PROTOCOL);
	int strsize1 = 0;

	memcpy(&strsize1, ptr, sizeof(strsize1));
	ptr += sizeof(strsize1);

	memcpy(str1, ptr, strsize1);
	ptr += strsize1;

	str1[strsize1] = '\0';
}

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
	char buf1[BUFSIZE],buf2[BUFSIZE];
	int len;
	int size;
	char packetbuf[BUFSIZE];
	bool endflag = false;

	// 서버와 데이터 통신
	while (1)
	{
		// 데이터 받기
		retval = recvn(sock, (char*)&size, sizeof(size), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("RecvError1()");
			break;

		}

		retval = recvn(sock, packetbuf, size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("RecvError2()");
			break;
		}

		PROTOCOL protocol;
		GetProtocol(packetbuf, protocol);

		switch (protocol)
		{
		case INTRO:
			UnPackPacket(packetbuf, buf1);
			printf("%s", buf1);

			scanf("%s", buf1);
			scanf("%s", buf2);

			PackPacket(packetbuf, LOGIN, buf1, buf2, size);

			retval = send(sock, packetbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("send()");
				continue;
			}
			break;

		case RESULT:
			UnPackPacket(packetbuf, buf1);
			printf("%s", buf1);
			endflag = true;
			break;
		}

		if (endflag)
		{
			break;
		}
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
