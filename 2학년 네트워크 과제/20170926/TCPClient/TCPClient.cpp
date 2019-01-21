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

	// ������ ������ ���
	while (1)
	{
		int size;
		char* ptr;

		RESULT result = NODATA;

		//��Ʈ�� ��ü ũ�� ����
		retval = recvn(sock, (char*)&size, sizeof(int), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		//��Ʈ�� ����
		retval = recvn(sock, buf, size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		PROTOCOL protocol;
		char intro[STRSIZE];		//��Ʈ�� ���� ���ڿ�
		int introsize;				//��Ʈ�� ���ڿ� ���� ���� ����
		ptr = buf;

		//���ۿ� ����ִ� ������ ��ü���� + �������� + ��Ʈ���� ���� + ��Ʈ�� ���ڿ�
		memcpy(&protocol, ptr, sizeof(PROTOCOL));
		
		if (protocol == INTRO)
		{
			ptr = ptr + sizeof(PROTOCOL);
			memcpy(&introsize, ptr, sizeof(int));
			ptr = ptr + sizeof(int);
			memcpy(intro, ptr, introsize);
		}		//����

		intro[introsize] = '\0';
		printf("%s\n", intro);
		//������ �ʱ�ȭ
		ZeroMemory(buf, sizeof(buf));
		
		//---------------------------------------------------------

		char ID[STRSIZE];
		char PW[STRSIZE];

		printf("���̵� �Է� : ");
		scanf("%s", ID);
		printf("��й�ȣ �Է� : ");
		scanf("%s", PW);

		protocol = ID_PW_INFO;
		size = sizeof(PROTOCOL);
		int idsize = strlen(ID) + 1;
		int pwsize = strlen(PW) + 1;

		//��ü����
		size = size + (sizeof(int)+idsize) + (sizeof(int)+pwsize);

		ptr = buf;
		memcpy(ptr, &size, sizeof(int));

		ptr = ptr + sizeof(int);
		memcpy(ptr, &protocol, sizeof(PROTOCOL));

		ptr = ptr + sizeof(PROTOCOL);
		memcpy(ptr, &idsize, sizeof(int));

		ptr = ptr + sizeof(int);
		memcpy(ptr, ID, idsize);

		ptr = ptr + sizeof(PROTOCOL);
		memcpy(ptr, &pwsize, sizeof(int));

		ptr = ptr + sizeof(int);
		memcpy(ptr, PW, pwsize);		//�ռ�

		//��ü���� + �������� + ���̵� ���ڿ� ���� + ���̵� ���ڿ� + ��й�ȣ ���ڿ� ���� + ��й�ȣ ���ڿ�

		//���̵�� ��й�ȣ�� ���ÿ� ����
		retval = send(sock, buf, sizeof(int)+size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}

		//---------------------------------------------------------

		//��� ��ü ũ�� ����
		retval = recvn(sock, (char*)&size, sizeof(int), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		//��� ����
		retval = recvn(sock, buf, size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		char str_result[STRSIZE];
		int str_result_size;

		ptr = buf;

		//��ü���� + �������� + ����� �ɺ� + ��� ���ڿ� ���� + ��� ���ڿ�

		memcpy(&protocol, ptr, sizeof(PROTOCOL));

		if (protocol == LOGIN_RESULT)
		{
			ptr = ptr + sizeof(PROTOCOL);
			memcpy(&result, ptr, sizeof(RESULT));

			ptr = ptr + sizeof(RESULT);
			memcpy(&str_result_size, ptr, sizeof(int));

			ptr = ptr + sizeof(int);
			memcpy(str_result, ptr, str_result_size);
		}	//����

		str_result[str_result_size] = '\0';
		printf("%s\n", str_result);		//���
		ZeroMemory(buf, sizeof(buf));

		if (result == LOGINSUCESS)		//������ Ż��
		{
			break;
		}
	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
