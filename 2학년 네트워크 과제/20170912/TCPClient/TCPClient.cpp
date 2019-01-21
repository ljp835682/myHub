#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"		
#define SERVERPORT 9000			
#define BUFSIZE    512
#define UP	3
#define OK 2
#define DOWN 1

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
	char buf[BUFSIZE + 1];
	int len;

	retval = recv(sock, buf, strlen(buf), 0);		//��Ʈ�� ���ڿ� ����
	buf[retval] = '\0';								//�ǳ��� �ι���
	
	int InputNum = 0;
	printf("%s", buf);				//���� ���ڿ� ���


	// ������ ������ ���
	while (1)
	{
		scanf("%d", &InputNum);		//�����Է¹���
		printf("\n");

		// ���� ������ ����
		retval = send(sock, (char *)&InputNum, sizeof(InputNum), 0);		//�������� ���ڸ� ����
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}

		//��� ����
		int Result;					//��� ������ ����
		retval = recvn(sock, (char *)&Result, sizeof(Result), 0);			//Result�� ������ ���� ����
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}

		bool Outflag = false;		//�ݺ��� Ż��� ����

		switch (Result)
		{
		case UP:					//���ϰ��
			printf("��!\n");		
			break;

		case DOWN:					//�ٿ��ϰ��
			printf("�ٿ�!\n");
			break;
				
		case OK:					//�����ϰ��
			printf("����!\n");
			
			retval = recv(sock, buf, strlen(buf), 0);		//������� ������� ���ڿ�����
			buf[retval] = '\0';								//�������� �ι���

			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}

			printf("%s\n", buf);							//���� ���ڿ� ���

			Outflag = true;
			break;
		}

		if (Outflag)
		{
			break;			//�ݺ��� Ż��
		}
	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}