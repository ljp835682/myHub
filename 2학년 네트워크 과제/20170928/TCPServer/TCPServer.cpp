#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 512
#define MAXUSER 100
#define PLAYERCOUNT 20

enum RESULT{ NODATA=0, IDERROR, PWERROR, LOGINSUCESS};
enum PROTOCOL{ INTRO, ID_PW_INFO, LOGIN_RESULT};

struct User_Info		//������ ������ ������ ����ü
{
	char ID[10];
	char PW[10];	
};

User_Info LoginInfo[3] = { { "aaa", "111" }, { "bbb", "222" }, { "ccc", "333" } };	//�̸� �����ص� 3���� ��������

struct sClientInfo		//Ŭ���̾�Ʈ�� ������ ������ ����ü
{
	SOCKET sock;				//����
	SOCKADDR_IN clientaddr;		//���Ͼִ�
	User_Info info;				//������ ����
	HANDLE hThread;				//�ڵ�
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
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");	
	
	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");
	
	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����		
	int addrlen;
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	
	while (1)
	{
		// accept()		
		addrlen = sizeof(clientaddr);
		sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (sock == INVALID_SOCKET)
		{
			err_display("accept()");
			continue;
		}

		sClientInfo* ptr = AddClient(sock, clientaddr);		//accept�� �����ϸ� ������ Ŭ�������� ����

		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(ptr->clientaddr.sin_addr), ntohs(ptr->clientaddr.sin_port));

		ptr->hThread = CreateThread(NULL, 0, ProcessClient,			//���ο� ������ �����ϰ�
			(LPVOID)ptr, 0, NULL);									//�� �����带 ProcessClient �� �Ҵ�

		if (ptr->hThread == NULL)					//���� ������ ����°� �����ϸ�
		{
			RemoveClient(ptr);						//Ŭ������ ����
		}

		else
		{
			CloseHandle(ptr->hThread);				//���ϸ���µ� �����ϸ� �˾Ƽ� ó���϶�� �Ǹ��� �����ع���
		}
	}

	// closesocket()
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}


// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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
	sClientInfo * NewClient = (sClientInfo*)arg;

	char* Intro = "���̵�� �н����带 �Է��ϼ���\n";
	char* ID_error = "���� ���̵��Դϴ�\n";
	char* PW_error = "�н����尡 Ʋ�Ƚ��ϴ�.\n";
	char* Login_Success = "�α��ο� �����߽��ϴ�.\n";

	int retval;
	int addrlen;

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(NewClient->clientaddr);
	//getpeername(NowUser->sock, (SOCKADDR *)&NowUser->clientaddr, &addrlen);		//�� ���Ͽ� ����� ���� ������ �ּҸ� �˾Ƴ��� �Լ�
																					//�ʿ���� ����
	PROTOCOL protocol = INTRO;					
	int size = sizeof(PROTOCOL);
	int intro_size = strlen(Intro);
	size = size + (sizeof(int) + intro_size);

	char* ptr = NewClient->buf;
	memcpy(ptr, &size, sizeof(int));

	ptr = ptr + sizeof(int);
	memcpy(ptr, &protocol, sizeof(PROTOCOL));

	ptr = ptr + sizeof(PROTOCOL);
	memcpy(ptr, &intro_size, sizeof(int));

	ptr = ptr + sizeof(int);
	memcpy(ptr, Intro, intro_size);			//Ŭ���̾�Ʈ���� ���� ���� ����
											//��ü���� + �������� + ��Ʈ�ι��ڿ��� ���� + ��Ʈ�ι��ڿ�

	// Ŭ���̾�Ʈ�� ������ ���
	retval = send(NewClient->sock, NewClient->buf, sizeof(int) + size, 0);	//������ ���� Ŭ���̾�Ʈ���� �۽�
	if (retval == SOCKET_ERROR)
	{
		err_display("send()");

		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(NewClient->clientaddr.sin_addr), ntohs(NewClient->clientaddr.sin_port));

		RemoveClient(NewClient);		//send ���н� Ŭ���̾�Ʈ ����
		return NULL;					//���� �Լ� ����
	}

	while (1)
	{
		RESULT result = NODATA;		

		int size;
		retval = recvn(NewClient->sock, (char*)&size, sizeof(int), 0);		//���� �������� ��ũ�⸸ �޾ƺ�
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		retval = recvn(NewClient->sock, NewClient->buf, size, 0);			//������ �˾Ƴ� ũ�⸸ŭ �����͸� �޾ƿ�
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		PROTOCOL protocol;
		int idsize;
		int pwsize;
		ptr = NewClient->buf;

		//���� �������� ����

		memcpy(&protocol, ptr, sizeof(PROTOCOL));					//��������		

		if (protocol == ID_PW_INFO)
		{
			ptr = ptr + sizeof(PROTOCOL);
			memcpy(&idsize, ptr, sizeof(int));						//���̵��� ����

			ptr = ptr + sizeof(int);
			memcpy(NewClient->info.ID, ptr, idsize);				//���̵� ���ڿ�

			ptr = ptr + idsize;
			memcpy(&pwsize, ptr, sizeof(int));						//�н������� ����

			ptr = ptr + sizeof(int);
			memcpy(NewClient->info.PW, ptr, pwsize);				//�н����� ���ڿ�
		}								

		// ���� ������ ���
		NewClient->info.ID[idsize] = '\0';
		NewClient->info.PW[pwsize] = '\0';
		printf("[TCP/%s:%d] ID: %s, PW:%s\n", inet_ntoa(NewClient->clientaddr.sin_addr),
			ntohs(NewClient->clientaddr.sin_port), NewClient->info.ID, NewClient->info.PW);
																	
		int result_size;
		char result_temp[BUFSIZE];
		for (int i = 0; i < sizeof(LoginInfo) / sizeof(User_Info); i++)		//����� ���������� ��ŭ �ݺ�
		{
			if (!strcmp(NewClient->info.ID, LoginInfo[i].ID))
			{
				if (!strcmp(NewClient->info.PW, LoginInfo[i].PW))
				{
					result = LOGINSUCESS;						//�α��� ����
					result_size = strlen(Login_Success);
					strcpy(result_temp, Login_Success);
				}
				else
				{
					result = PWERROR;							//�н����� �̽���Ī
					result_size = strlen(PW_error);
					strcpy(result_temp, PW_error);
				}
				break;
			}
		}

		if (result == NODATA)
		{
			result = IDERROR;									//���̵� ��й�ȣ �̽���Ī				
			result_size = strlen(ID_error);
			strcpy(result_temp, ID_error);
		}


		ptr = NewClient->buf;									//�ʱ�ȭ
		protocol = LOGIN_RESULT;								//�α��� ��� ���ڿ��� ���ۿ� ����
		size = sizeof(PROTOCOL) + sizeof(RESULT) + sizeof(int) + result_size;		//��ũ��
		memcpy(ptr, &size, sizeof(int));

		ptr = ptr + sizeof(int);
		memcpy(ptr, &protocol, sizeof(PROTOCOL));			

		ptr = ptr + sizeof(PROTOCOL);
		memcpy(ptr, &result, sizeof(RESULT));


		ptr = ptr + sizeof(RESULT);
		memcpy(ptr, &result_size, sizeof(int));


		ptr = ptr + sizeof(int);
		memcpy(ptr, result_temp, result_size);			//Ŭ���̾�Ʈ���� ���� ���� ����
														//��ü���� + �������� + ������ڿ��� ���� + ������ڿ�


		// ������ ������
		retval = send(NewClient->sock, NewClient->buf, sizeof(int) + size, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	// closesocket()
	closesocket(NewClient->sock);
	printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(NewClient->clientaddr.sin_addr), ntohs(NewClient->clientaddr.sin_port));

	RemoveClient(NewClient);
	return 0;
}


sClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr)
{
	sClientInfo* ptr = new sClientInfo;
	ZeroMemory(ptr, sizeof(sClientInfo));
	ptr->sock = sock;
	strcpy_s(ptr->info.ID, strlen("") + 1, "");
	strcpy_s(ptr->info.PW, strlen("") + 1, "");
	memcpy(&(ptr->clientaddr), &clientaddr, sizeof(clientaddr));
	ClientInfo[count] = ptr;
	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));
	count++;
	return ptr;
}

void RemoveClient(sClientInfo* ptr)
{
	closesocket(ptr->sock);

	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(ptr->clientaddr.sin_addr),
		ntohs(ptr->clientaddr.sin_port));

	for (int i = 0; i < count; i++)
	{
		if (ClientInfo[i] == ptr)
		{
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