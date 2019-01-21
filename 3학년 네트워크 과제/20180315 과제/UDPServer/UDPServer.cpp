#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512
#define SIMBOL 0
#define ERRSIMBOL -1

#define PARK_ROOM "233.3.3.3"		
#define O_ROOM "233.3.3.4"		
#define KIM_ROOM "233.3.3.5"		

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

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");

	// ������ ��ſ� ����� ����
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE+1] = "";
	bool flag = false;

	// Ŭ���̾�Ʈ�� ������ ���
	while(1){
		// ������ �ޱ�
		addrlen = sizeof(clientaddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR *)&clientaddr, &addrlen);
		if(retval == SOCKET_ERROR){
			err_display("recvfrom()");
			continue;
		}

		switch (atoi(buf))
		{
		case 0:
			// ���ۿ� ���� �Է�
			strcpy_s(buf, "ä�ù��� �����ϼ���\n1. �ڿ���\n2. ����ȯ\n3. ������\n ��� : ");

			// ���� ������
			retval = sendto(sock, buf, strlen(buf), 0,
				(SOCKADDR *)&clientaddr, sizeof(clientaddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}

			flag = false;
			break;
		case 1:
			strcpy_s(buf, PARK_ROOM);	
			flag = true;
			break;
		case 2:	
			strcpy_s(buf, O_ROOM);		//�´� �ּҸ� �Է½�Ŵ
			flag = true;
			break;
		case 3:
			strcpy_s(buf, KIM_ROOM);
			flag = true;
			break;
		default:
			strcpy_s(buf, "-1");		//�׿��ϰ�� �����ɺ��� �����Ұ�
			flag = true;
			break;
		}

		// Ŭ�� ������ ���� �ּҸ� ����
		if (flag)
		{
			retval = sendto(sock, buf, strlen(buf), 0,
				(SOCKADDR *)&clientaddr, sizeof(clientaddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}
		}
		
		// ���� �ʱ�ȭ
		ZeroMemory(buf, BUFSIZE + 1);
	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}

//UDP�� ����� ä��â �����
//������ Ŭ���̾�Ʈ�� ����
//�������� Ŭ���̾�Ʈ�� ä�ÿ�û ��Ŷ�� ����
//������ �� ��Ŷ���� �� Ŭ���̾�Ʈ�� �ּҸ� �˰ԵǸ�
//�� Ŭ���̾�Ʈ����
//ä�ù��� �����ϼ���
//1. �ڿ��ù�
//2. ����ȯ��
//3. ��������
//��� �޼����� ������
//�� Ŭ���̾�Ʈ�� ���� ��������
//������ 224.0.0.0 ~ 235.255.255.255 �� �ּҸ� ����ϰ��ִٰ�
//�´� ���� �ּҸ� Ŭ�󿡰� ������
//Ŭ��� �׹濡 ����