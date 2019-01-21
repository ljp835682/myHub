#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;
	
	int optval, optlen;

	// socket()
	SOCKET udpsock = socket(AF_INET, SOCK_DGRAM, 0);
	if(udpsock == INVALID_SOCKET) err_quit("socket()");

	// socket()
	SOCKET tcpsock = socket(AF_INET, SOCK_STREAM, 0);
	if (tcpsock == INVALID_SOCKET) err_quit("socket()");

	for(int i=0;i<17;i++)
	{
		// ���� ������ ũ�⸦ ��´�.
		optlen = sizeof(optval);
		retval = getsockopt(udpsock, SOL_SOCKET, SO_RCVBUF,
			(char *)&optval, &optlen);
		if (retval == SOCKET_ERROR) err_quit("getsockopt()");
		printf("���� ���� ũ��(old) = %d����Ʈ\n", optval);

		// ���� ������ ũ�⸦ �� ��� �ø���.
		optval *= 2;
		retval = setsockopt(udpsock, SOL_SOCKET, SO_RCVBUF,
			(char *)&optval, sizeof(optval));
		if (retval == SOCKET_ERROR) err_quit("setsockopt()");

		//�۽� ������ ũ�⸦ ��´�.
		retval = getsockopt(udpsock, SOL_SOCKET, SO_SNDBUF,
			(char *)&optval, &optlen);
		if (retval == SOCKET_ERROR) err_quit("getsockopt()");
		printf("���� ���� ũ��(old) = %d����Ʈ\n", optval);

		// �۽� ������ ũ�⸦ �� ��� �ø���.
		optval *= 2;
		retval = setsockopt(udpsock, SOL_SOCKET, SO_SNDBUF,
			(char *)&optval, sizeof(optval));
		if (retval == SOCKET_ERROR) err_quit("setsockopt()");

		// ���� ������ ũ�⸦ ��´�.
		optlen = sizeof(optval);
		retval = getsockopt(tcpsock, SOL_SOCKET, SO_RCVBUF,
			(char *)&optval, &optlen);
		if (retval == SOCKET_ERROR) err_quit("getsockopt()");
		printf("���� ���� ũ��(old) = %d����Ʈ\n", optval);

		// ���� ������ ũ�⸦ �� ��� �ø���.
		optval *= 2;
		retval = setsockopt(tcpsock, SOL_SOCKET, SO_RCVBUF,
			(char *)&optval, sizeof(optval));
		if (retval == SOCKET_ERROR) err_quit("setsockopt()");

		//�۽� ������ ũ�⸦ ��´�.
		retval = getsockopt(tcpsock, SOL_SOCKET, SO_SNDBUF,
			(char *)&optval, &optlen);
		if (retval == SOCKET_ERROR) err_quit("getsockopt()");
		printf("���� ���� ũ��(old) = %d����Ʈ\n", optval);

		// �۽� ������ ũ�⸦ �� ��� �ø���.
		optval *= 2;
		retval = setsockopt(tcpsock, SOL_SOCKET, SO_SNDBUF,
			(char *)&optval, sizeof(optval));
		if (retval == SOCKET_ERROR) err_quit("setsockopt()");
	}

	// closesocket()
	closesocket(udpsock);

	// ���� ����
	WSACleanup();
	return 0;
}