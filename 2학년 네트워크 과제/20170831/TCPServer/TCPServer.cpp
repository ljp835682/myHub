#pragma comment(lib, "ws2_32")		//���̺귯�� ���� "ws2_32"�� ���Խ�Ų�ٴ� ��ó����

									//lib�� ���� ���̺귯�� �̸� �̹� ������ ���̺귯���� ���Խ�Ű�°�
									//ó������ ���ԵǾ��ֱ⿡ ������� exe������ �뷮�� Ŀ���� ������ ������

									//dll�� ���� ���̺귯�� �ʿ��Ҷ� ���� ����
									//ó������ ���� ����ǰ� ������� exe���ϵ� �뷮�� ������ dll������ ȣ���Ҷ� ��������
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)												//���̻� ������ �Ұ����� ���α׷��� �����ؾ��ҋ� ȣ���ϴ� �Լ�
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
���� err_display �� ��������� ���ư����� �����ؿð�
*/

// ���� �Լ� ���� ���
void err_display(char *msg)												//������ ���� ������ �����Ҷ� ���ڸ� ����� �Լ�
{
	LPVOID lpMsgBuf;		//���̵� ������
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);				//���ڿ� �����ͷ� ��ȯ�Ͽ� �Լ��� ������ lpMsgBuf��� ���̵������Ϳ� ��� ���ڿ��� �ּҸ� �������
	printf("[%s] %s", msg, (char *)lpMsgBuf);		//���̵������͸� ĳ���� �����ͷ� �������ȯ�ؼ� ���
	LocalFree(lpMsgBuf);
}

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;									//������ ���� ������
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)		//2.2������ ��������� dll���̺귯���� �ʱ�ȭ��Ű�µ� ����ε��� �������
		return 1;									//���α׷� ����

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);		//socket(IPv4 �� ���,TCP �������� ���,�׻� 0) �Լ��Ϸ��� �ڵ�(������+�ĺ���ȣ)�� ����
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");		//���� ����Ҽ� ���� ����(�ȸ��������� ��)�ϰ�� err_quit�Լ� ȣ��

	// bind()
	SOCKADDR_IN serveraddr;									
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;				//
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);	//�ּ����� ����
	serveraddr.sin_port = htons(SERVERPORT);		//
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));	//���ε��� ���Ϲ�ȣ�� ���Ͽ� �Ҵ��Ѵ�
	if(retval == SOCKET_ERROR) err_quit("bind()");

	// listen()											//���������� ����ϴ� �Լ�	(�������� ó��������� ������ �����û���� �޴� �����̵Ǹ� �̸� ��������,�������� �̶� �θ���)
	retval = listen(listen_sock, SOMAXCONN);			//���������� �������ִ� �ִ�ũ��
	if(retval == SOCKET_ERROR) err_quit("listen()");	//���������� ��������� ������� ����

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE+1];

	while(1)
	{
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);		//accept -> �޴��Լ�
																					//accept�� ���������� �޾Ƴ��� �����û��Ŷ�� �����ͼ� �� ��Ŷ�̶� ������ ������ ���� �׶��� ��Ʈ�� ���������
																					//(SOCKADDR *)&clientaddr���� �̹��� ���� ���Ͽ� ����� Ŭ���̾�Ʈ�� ������ ��Ƶд�
		if(client_sock == INVALID_SOCKET)											//���������� �����û ������ ������������ accept�� ��(����)�ȴ�
		{
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// Ŭ���̾�Ʈ�� ������ ���
		while(1)
		{
			// ������ �ޱ�															//recv -> ���ú��Լ�
			retval = recv(client_sock, buf, BUFSIZE, 0);							//recv(�����͸� ���� ����,����,������ �ִ�ũ��,������ 0)

			if(retval == SOCKET_ERROR)												//�������� Ȯ��
			{
				err_display("recv()");
				break;
			}
			else if(retval == 0)													//���� 0�ϰ�� Ŭ���� �����̶�� ���̹Ƿ� �� �ݺ����� ����
				break;

			// ���� ������ ���
			buf[retval] = '\0';														//���ú� �Լ��� ���� �����ʹ� �������� ���� �����Ƿ� �����Ⱚ���� �ƴ��� �˼����⿡ �������� ���� �־��ش�
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),				//������ ���
				ntohs(clientaddr.sin_port), buf);

			// ������ ������
			retval = send(client_sock, buf, retval, 0);								//�޾����ϱ� TCP���������� ��Ģ�� ���Ͽ� �ٽ� ��������ϹǷ� ���� �Լ��� ȣ���Ѵ� 
			if(retval == SOCKET_ERROR)
			{
				err_display("send()");
				break;
			}
		}

		// closesocket()
		closesocket(client_sock);													//closesocket�Լ��� ������ �ݴ¿뵵�� �Լ�
																					//��ȭ�� ������ ����� ������ �ݴ´�
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",			//���
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket(listen_sock);														//���α׷� ������ ���� ���� �ݱ�

	// ���� ����
	WSACleanup();																	//dll ����
	return 0;
}