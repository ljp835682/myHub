#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "User.h"

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

/*
���� err_display �� ��������� ���ư����� �����ؿð�
*/

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID lpMsgBuf;		//���̵� ������
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

	// ���� �ʱ�ȭ
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

	// ������ ��ſ� ����� ����
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

		// ������ Ŭ���̾�Ʈ ���� ���
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// ��Ʈ�� �۽�
		send(client_sock, Intro, strlen(Intro), 0);

		// Ŭ���̾�Ʈ�� ������ ���
		while (1)
		{			
			//���� ��ü ����
			cUser * Temp = new cUser();
			retval = recv(client_sock, (char*)Temp, BUFSIZE, 0);	//��ü�� ���ڿ� ũ�� �˼����� recv ���

			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}

			else if (retval == 0)
			{
				break;
			}

			// ���� ��ü�� ���� ������ ��
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

			//Ŭ���̾�Ʈ�� �Է� ���� ����
			delete Temp;

			//��� �۽�
			send(client_sock, (char *)&Msgflag, sizeof(Msgflag), 0);

			//�α��μ����� Ŭ���̾�Ʈ ���� ����
			if (Msgflag == OK)
			{
				break;
			}
		}

		// closesocket()
		closesocket(client_sock);

		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}
	//�������� ����
	delete User;

	// closesocket()
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}

/*

�α��μ��� �����
Ŭ�� �������� ������ ���̵�� �н����� �Է�
������ �̹� � ���̵�� ��й�ȣ�� �������ְ�
���̵𰡾����� ���̵� ���ٰ� �˷��ְ�
�н����尡 �ٸ��� �н����尡 �ٸ��ٰ� �޷��ְ�
�α��εǸ� �α��μ���
�α����� �����Ұ�� �ٽ� ��� �α��νõ�
ȸ���� ������ ����ü�� �����ؼ� �ѹ��� ����
�Ȱ��� ������ ����ü�� Ŭ��� ���� �Ѵ� �������վ����

*/