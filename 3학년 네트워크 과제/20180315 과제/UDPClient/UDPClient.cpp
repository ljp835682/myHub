#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000					//�����ϰ� ������ ��Ʈ
#define BUFSIZE    512
#define REMOTEPORT  9003				//Ŭ�󳢸� ������ ��Ʈ
#define LOCALPORT   9003				//Ŭ�󳢸� ������ ��Ʈ
#define SIMBOL 0
#define ERRSIMBOL -1

DWORD WINAPI RecvThread(LPVOID arg);	//�б� ������

char address[100] = "";					//Ŭ���̾�Ʈ�� �� ��Ƽĳ���ÿ� �ּҸ� ������ ����

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

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// ���� �ּ� ����ü �ʱ�ȭ
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);

	// ������ ��ſ� ����� ����
	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[BUFSIZE+1] = "0";
	int len;
	int choice;

	// ������ ������ ���
	while(1)
	{
		// �ɺ� ������
		retval = sendto(sock, buf, strlen(buf), 0,
			(SOCKADDR *)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}

		// ���� �ޱ�
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}

		// �ι��� ����
		buf[retval] = '\0';

		// ���� ���	
		printf("%s", buf);
		ZeroMemory(buf, BUFSIZE + 1);

		// ���� �ۼ��� �ɺ� �Է� �Ұ��ϵ��� ����
		while (1)
		{
			scanf("%d", &choice);
			if (choice != SIMBOL && choice != ERRSIMBOL)
			{
				break;
			}

			else
			{
				printf("ERR!!\n");
			}
		}
		

		sprintf(buf, "%d", choice);

		// ��� ������
		retval = sendto(sock, buf, strlen(buf), 0,
			(SOCKADDR *)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}

		// ���� �ּ� Ȯ���ϱ�
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}

		// ���� �ּ� ����
		strcpy_s(address, buf);
		ZeroMemory(buf, BUFSIZE + 1);

		// ��Ƽĳ��Ʈ �׷� ���� ���н�
		if (atoi(address) == ERRSIMBOL)
		{
			printf("ERR!!\n");
		}

		// ��Ƽĳ��Ʈ �׷� ���� ������
		else
		{
			// ���ú� ������ ����
			HANDLE hThread = CreateThread(NULL, 0, RecvThread, NULL, 0, NULL);
			if (hThread == NULL) return 1;
			CloseHandle(hThread);

			// ��Ƽĳ��Ʈ Time-to-live ����
			int ttl = 2;
			retval = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
				(char *)&ttl, sizeof(ttl));

			if (retval == SOCKET_ERROR) err_quit("setsockopt()");

			// ���� �ּ� ����ü �ʱ�ȭ
			SOCKADDR_IN remoteaddr;
			ZeroMemory(&remoteaddr, sizeof(remoteaddr));
			remoteaddr.sin_family = AF_INET;
			remoteaddr.sin_addr.s_addr = inet_addr(address);
			remoteaddr.sin_port = htons(REMOTEPORT);

			// ���� �ʱ�ȭ
			ZeroMemory(buf, BUFSIZE + 1);

			// ���� �Է��� ���� �ݺ���
			while (1)
			{
				// ������ �Է�
				printf("\n[���� ������] ");
				scanf("%s", buf);

				// '\n' ���� ����
				len = strlen(buf);
				if (buf[len - 1] == '\n')
					buf[len - 1] = '\0';
				if (strlen(buf) == 0)
					break;

				// ������ ������
				retval = sendto(sock, buf, strlen(buf), 0,
					(SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
				if (retval == SOCKET_ERROR) {
					err_display("sendto()");
					continue;
				}
			}
		}
	}

	// ���� ����
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}

DWORD WINAPI RecvThread(LPVOID arg)
{
	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// SO_REUSEADDR �ɼ� ����
	BOOL optval = TRUE;
	int retval = setsockopt(sock, SOL_SOCKET,
		SO_REUSEADDR, (char *)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// ���ε�
	SOCKADDR_IN localaddr;
	ZeroMemory(&localaddr, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(LOCALPORT);
	retval = bind(sock, (SOCKADDR *)&localaddr, sizeof(localaddr));
	if (retval == SOCKET_ERROR)
	{
		err_quit("bind()");
	}

	// ��Ƽĳ��Ʈ �׷� ����
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(address);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	retval = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// ������ ��ſ� ����� ����
	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	// ��Ƽĳ��Ʈ ������ �ޱ�
	while (1) {
		// ������ �ޱ�
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}

		// ���� ������ ���
		buf[retval] = '\0';
		printf("[UDP/%s:%d] %s\n", inet_ntoa(peeraddr.sin_addr),
			ntohs(peeraddr.sin_port), buf);
	}

	// ��Ƽĳ��Ʈ �׷� Ż��
	retval = setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");


	return 0;
}