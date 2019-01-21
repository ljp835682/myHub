#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "User.h"

#define SERVERIP   "127.0.0.1"		
#define SERVERPORT 9000			
#define BUFSIZE    512

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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

// ����� ���� ������ ���� �Լ�
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
	// ���� �ʱ�ȭ
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

	// ������ ��ſ� ����� ����
	char buf[BUFSIZE + 1] = " ";
	int len;

	// ��Ʈ�� ����
	recv(sock, buf, BUFSIZE, 0);	//���ڿ� ���� �������� ũ�� �˼����� recv���		
	buf[strlen(buf) + 1] = '\0';				
	printf("%s\n", buf);

	// ������ ������ ���
	while (1) {

		//���̵��Է�
		printf("ID : ");
		scanf("%s", ID);
		printf("PW : ");
		scanf("%s", PW);

		User->SetIDPW(ID, PW);

		// ���� ��ü �۽�
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

		//��� ����
		int Msgflag = 0;
		retval = recvn(sock, (char *)&Msgflag, sizeof(Msgflag), 0);		//�̸� ����ص� �ɺ� ���� �������� ũ�� �˼����� recvn���
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}

		//����� ��
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

		//�޼��� ���
		printf("%s\n", Message);

		//������ ó��
		if (Msgflag == OK)
		{
			delete User;
			break;
		}
	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
