#pragma comment(lib, "ws2_32")		//���̺귯�� ���� "ws2_32"�� ���Խ�Ų�ٴ� ��ó����
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 512
#define MAXUSER 100
#define PLAYERCOUNT 20

enum RESULT { NODATA = 0, WIN, LOOSE, DRAW };
enum PROTOCOL { INTRO, INPUTDATA, GAME_RESULT };

struct User_Info		//������ ������ ������ ����ü
{
	char * Result;
	int HandState;
};

struct sClientInfo		//Ŭ���̾�Ʈ�� ������ ������ ����ü
{
	SOCKET sock;				//����
	SOCKADDR_IN clientaddr;		//���Ͼִ�
	User_Info info;				//������ ����
	sClientInfo * Partner;		//��Ʈ�� Ŭ���̾�Ʈ
	HANDLE hThread;				//�ڵ�
	int index;					//�����ĺ���ȣ
	char buf[BUFSIZE + 1];		//����
};

sClientInfo* ClientInfo[PLAYERCOUNT];		//Ŭ���̾�Ʈ ���� ����ü ������ �迭
int count = 0;								//ī��Ʈ

sClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr);		//Ŭ���̾�Ʈ �߰� �Լ�
void RemoveClient(sClientInfo* ptr);								//Ŭ���̾�Ʈ ���� �Լ�
void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);
DWORD WINAPI ProcessClient(LPVOID arg);								//�����尡 ������ �Լ�				

int main(int argc, char* argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����		
	int addrlen;
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	while (1)
	{
		//accept()
		sClientInfo * NewPlayer;

		addrlen = sizeof(clientaddr);
		sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);									//���ο� ���� ����
		if (sock == INVALID_SOCKET)
		{
			err_display("accept()");
			continue;
		}

		NewPlayer = AddClient(sock, clientaddr);														//�߰�

		NewPlayer->hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)NewPlayer, 0, NULL);			//������ �Լ� ����

		if (NewPlayer->hThread == NULL)
		{
			RemoveClient(NewPlayer);																		//���н� ���� ����
		}

		else
		{
			CloseHandle(NewPlayer->hThread);																//������ ������ �Լ� ���� ����
		}
	}

	// closesocket()
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}

void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
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


// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClient(LPVOID arg)
{
	//--------------------------------------------------------------------------------------------------------------
	//��Ʈ�� �۽�

	char* Intro = "0. ���� 1. ���� 2. ��\n";

	sClientInfo * Player = (sClientInfo*)arg;							//����

	int retval = 0;

	PROTOCOL protocol = INTRO;
	int size = sizeof(PROTOCOL);
	int intro_size = strlen(Intro);
	size = size + (sizeof(intro_size) + intro_size);					//��ü ������

	char* ptr = Player->buf;
	memcpy(ptr, &size, sizeof(size));

	ptr = ptr + sizeof(size);
	memcpy(ptr, &protocol, sizeof(protocol));

	ptr = ptr + sizeof(protocol);
	memcpy(ptr, &intro_size, sizeof(intro_size));

	ptr = ptr + sizeof(intro_size);
	memcpy(ptr, Intro, intro_size);										//��ü ������ + �������� + ��Ʈ�� ������ + ��Ʈ�� ���ڿ�

	retval = send(Player->sock, Player->buf, sizeof(int) + size, 0);	//������ ���� Ŭ���̾�Ʈ���� �۽�
	if (retval == SOCKET_ERROR)
	{
		err_display("IntroSend()");

		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(Player->clientaddr.sin_addr), ntohs(Player->clientaddr.sin_port));

		RemoveClient(Player);											//send ���н� Ŭ���̾�Ʈ ����
		return NULL;
	}

	RESULT result = NODATA;

	//--------------------------------------------------------------------------------------------------------------
	//Ŭ���̾�Ʈ�� ���ð� ����

	size = 0;
	retval = recvn(Player->sock, (char*)&size, sizeof(int), 0);		//���� �������� ��ũ��

	if (retval == SOCKET_ERROR)
	{
		err_display("recv()");
		{
			RemoveClient(Player);
			return NULL;
		}
	}

	retval = recvn(Player->sock, Player->buf, size, 0);				//�˾Ƴ� ũ�⸸ŭ �����͸� ����

	if (retval == SOCKET_ERROR)
	{
		err_display("recv()");
		{
			RemoveClient(Player);
			return NULL;
		}
	}

	ptr = Player->buf;

	//���� �������� ����

	memcpy(&protocol, ptr, sizeof(protocol));					//��������		

	if (protocol == INPUTDATA)
	{
		ptr = ptr + sizeof(protocol);
		memcpy(&Player->info.HandState, ptr, sizeof(Player->info.HandState));		//Ŭ�� ������ ��
	}

	//--------------------------------------------------------------------------------------------------------------
	//��� ���

	// ���� ������ ���
	printf("[TCP/%s:%d] %d�� �÷��̾� ����: %d\n", inet_ntoa(Player->clientaddr.sin_addr), ntohs(Player->clientaddr.sin_port), Player->index, Player->info.HandState);
	
	while (1)
	{
		if (Player->Partner != nullptr)						//��Ʈ�ʰ� ������ ���涧���� ���
		{
			if (Player->Partner->info.HandState != -1)		//���� ��Ʈ�ʰ� �����ʾ������ ���
			{
				break;
			}
		}
	}

	//�º� ���
	Player->info.Result = (Player->info.HandState == Player->Partner->info.HandState) ? "Draw" : (((Player->info.HandState + 1) % 3 == Player->Partner->info.HandState) ? "You Lose!" : "You Win!");

	
	printf("�÷��̾�%d : %s\n", Player->index, Player->info.Result);		//���������� ���


	//-------------------------------------------------------------------------------------------------------------
	//��� �۽�

	protocol = GAME_RESULT;							
	size = sizeof(protocol);
	int game_result_size = strlen(Player->info.Result);
	size = size + (sizeof(game_result_size) + game_result_size);						//��ü ������

	ptr = Player->buf;
	memcpy(ptr, &size, sizeof(size));

	ptr = ptr + sizeof(size);
	memcpy(ptr, &protocol, sizeof(protocol));

	ptr = ptr + sizeof(protocol);
	memcpy(ptr, &game_result_size, sizeof(game_result_size));

	ptr = ptr + sizeof(game_result_size);
	memcpy(ptr, Player->info.Result, game_result_size);					//��ü ������ + �������� + ��� ���ڿ� ������ + ��� ���ڿ�

	retval = send(Player->sock, Player->buf, sizeof(int) + size, 0);	//������ ���� Ŭ���̾�Ʈ���� �۽�
	if (retval == SOCKET_ERROR)
	{
		err_display("game_result Send()");
	}

	printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(Player->clientaddr.sin_addr), ntohs(Player->clientaddr.sin_port));
	RemoveClient(Player);

	return 0;
}


sClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr)
{
	sClientInfo* ptr = new sClientInfo;
	ZeroMemory(ptr, sizeof(sClientInfo));
	ptr->sock = sock;
	ptr->info.HandState = -1;

	for (int i = 0; i < count; i++)
	{
		if (ClientInfo[i]->Partner == nullptr)		//��Ʈ�ʰ� ���� Ŭ�� �߽߰�
		{
			ClientInfo[i]->Partner = ptr;			//���� �߰� Ŭ�� �߰� Ŭ��� ��Ʈ�� ����
			ptr->Partner = ClientInfo[i];			//�߰� Ŭ�� ���� �߰� Ŭ��� ��Ʈ�� ����
			break;									//�ߺ� ������ ���� �ʵ��� break
		}
	}

	memcpy(&(ptr->clientaddr), &clientaddr, sizeof(clientaddr));
	ClientInfo[count] = ptr;
	ClientInfo[count]->index = count;				//�ĺ���ȣ
	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));
	count++;
	return ptr;
}

void RemoveClient(sClientInfo* ptr)
{
	closesocket(ptr->sock);

	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(ptr->clientaddr.sin_addr), ntohs(ptr->clientaddr.sin_port));

	for (int i = 0; i < count; i++)
	{
		if (ClientInfo[i] == ptr)					
		{
			if (ptr->Partner != nullptr)				//������ Ŭ���� ��Ʈ�ʰ� �������Ͱ� �ƴѰ��
			{
				ptr->Partner->Partner = nullptr;		//������ Ŭ���� ��Ʈ�ʰ� �ڽ��� ����Ű�°� �������͸� ����Ű�� �ٲ���
			}
			
			delete ptr;
			int j;
			for (j = i; j < count - 1; j++)
			{
				ClientInfo[j] = ClientInfo[j + 1];
			}
			ClientInfo[j] = nullptr;
			break;
		}
	}
	count--;
}