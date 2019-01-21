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

struct User_Info		//������ ������ ������ ����ü
{
	char * Result;
	int HandState;
};


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
		retval = recvn(sock, (char*)&size, sizeof(int), 0);		//��ü ���� ������ �˾ƺ�
		if (retval == SOCKET_ERROR)
		{
			err_quit("recv()");
		}
		else if (retval == 0)
		{
			return 0;
		}

		retval = recvn(sock, (char*)buf, size, 0);				//�˾Ƴ� �����ŭ ����
		if (retval == SOCKET_ERROR)
		{
			err_quit("recv()");
		}
		else if (retval == 0)
			return 0;

		PROTOCOL protocol;
		char* ptr = buf;										

		memcpy(&protocol, ptr, sizeof(PROTOCOL));				//��������
		ptr = ptr + sizeof(PROTOCOL);							//���������� ũ�⸸ŭ �̵�

		// ������ ��ſ� ����� ����
		bool endflag = false;

		switch (protocol)
		{
		case INTRO:												//��Ʈ���� ���

			int intro_size;
			memcpy(&intro_size, ptr, sizeof(int));				//��Ʈ���� ���ڿ� ����
			ptr = ptr + sizeof(intro_size);						

			char intro[BUFSIZE];
			memcpy(intro, ptr, intro_size);						//��Ʈ�� ���ڿ�
			ptr = ptr + intro_size;

			intro[intro_size] = '\0';							//���� �ι���
			printf("%s\n", intro);								//���
			break;

		case GAME_RESULT:

			endflag = true;										//�ݺ��� Ż��� �÷���

			int game_result_size;
			memcpy(&game_result_size, ptr, sizeof(int));		//��� ���ڿ� ����
			ptr = ptr + sizeof(int);

			char game_result[BUFSIZE];
			memcpy(game_result, ptr, game_result_size);			//��� ���ڿ�
			ptr = ptr + game_result_size;

			game_result[game_result_size] = '\0';				//���� �ι���
			printf("%s\n", game_result);						//���
			break;
		}

		if (endflag)
		{
			break;												//GAME_RESULT ������� ����
		}

		User_Info info;
		ZeroMemory(&info, sizeof(info));
		printf("�Է� :");										//Ŭ���� �Է�
		scanf("%d",&info.HandState);							

		ptr = buf;
		protocol = INPUTDATA;									
		size = sizeof(size)+sizeof(protocol)+sizeof(info.HandState);

		memcpy(ptr, &size, sizeof(size));
		ptr = ptr + sizeof(size);

		memcpy(ptr, &protocol, sizeof(protocol));
		ptr = ptr + sizeof(protocol);

		memcpy(ptr, &info.HandState, sizeof(info.HandState));
		ptr = ptr + sizeof(info.HandState);								//��ü ������ + �������� + ������ ����

		retval = send(sock, buf, sizeof(int)+size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}

	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
