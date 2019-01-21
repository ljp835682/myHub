#pragma comment(lib, "ws2_32")
#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define BUFSIZE 512
#define PLAYERCOUNT 10
#define NODATA -1

enum { NO_WIN, FIRST_WIN, SECOND_WIN = -1 };
enum STATE{ INTRO_SEND, GVALUE_RECV, RESULT_SEND, GAME_OVER, DISCONNECT };
enum PROTOCOL{ INTRO, GVALUE, RESULT };

struct _ClientInfo
{
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	STATE state;
	int	gvalue;
	HANDLE thread;
	char  packetbuf[BUFSIZE];
	_ClientInfo* part;
};

HANDLE hThread[PLAYERCOUNT + 1];
_ClientInfo* ClientInfo[PLAYERCOUNT];
int count = 0;
int th_count = 0;

int who_win(int part1, int part2);
void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);
_ClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr);
void RemoveClient(_ClientInfo* ptr);
bool MatchPartner(_ClientInfo* _ptr);
void AddThread(_ClientInfo* _ptr);
void RemoveThread(int index);
bool SendFunction(char * _str, _ClientInfo* _player, PROTOCOL _protocol);
bool RecvFunction(_ClientInfo * _player);
DWORD CALLBACK ProcessClient(LPVOID);
DWORD WINAPI Dummy(LPVOID arg);
DWORD WINAPI WaitingThread(LPVOID arg);


char No_win[] = "�����ϴ�.!!";
char win[] = "�̰���ϴ�!!";
char lose[] = "�����ϴ�!!!!";
char Intro[] = "����:0, ����:1, ��:2 �� �Է��ϼ���:";

int main(int argc, char **argv)
{
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
	int retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����		
	int addrlen;
	SOCKET sock;
	SOCKADDR_IN clientaddr;

	hThread[th_count++] = CreateThread(NULL, 0, Dummy, 0, 0, NULL);								//���� ������ �Լ� ����
	HANDLE wating = CreateThread(NULL, 0, WaitingThread, 0, 0, NULL);					//���ÿ� ������ �Լ� ����
	CloseHandle(wating);																//���ÿ� ������ �Լ� �Ǹ� ����

	while (1)
	{
		addrlen = sizeof(clientaddr);
		sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (sock == INVALID_SOCKET)
		{
			err_display("accept()");
			continue;
		}

		_ClientInfo* ptr = AddClient(sock, clientaddr);
		MatchPartner(ptr);
		AddThread(ptr);
		
		if (ptr->thread == NULL)
		{
			RemoveClient(ptr);
			continue;
		}

		else
		{			
			TerminateThread(hThread[0], 0);												//���� ������ �Լ� �͹̳���Ʈ
			th_count++;
		}
	}

	closesocket(listen_sock);
	
	WSACleanup();
	return 0;
}

DWORD WINAPI WaitingThread(LPVOID arg)
{
	while (1)
	{
		int index = WaitForMultipleObjects(th_count, hThread, false, INFINITE);		//th_count��ŭ ���� ������ �Լ��� �����Ѵ�

		if (index == 0)																//���� ���� ������ �Լ��� �׾��ٸ�
		{
			hThread[0] = CreateThread(NULL, 0, Dummy, 0, 0, NULL);					//���ο� �Լ��� ���°���
		}																			//�ٽ� WaitForMultipleObjects �� Ȱ���� �ϱ����� ���� ������ �Լ��� ������ش�

		else if (index == -1)
		{
			printf("index ���� -1 �����߻�\n");
			continue;
		}

		else
		{
			printf("%d�� ������ ����\n", index);									//�׿ܿ��� ���� ������ �Լ��� ����Ȱ�

			_ClientInfo * ptr = nullptr;											

			for (int i = 0; i < count; i++)
			{
				if (ClientInfo[i]->thread == hThread[index])
				{
					ptr = ClientInfo[i];
					break;
				}
			}

			if (ptr == nullptr)
			{
				printf("����� ������� ���������� ������ Ŭ���̾�Ʈ�� ������������(����)\n");
				continue;
			}

			switch (ptr->state)
			{
			case GAME_OVER:
				RemoveClient(ptr);
				break;
			case RESULT_SEND:
				
				break;
			case DISCONNECT:
				if (ptr->part != nullptr)
				{
					if (MatchPartner(ptr->part))
					{
						if (ptr->part == nullptr)
						{
							AddThread(ptr->part);
						}

						if (ptr->part->part == nullptr)
						{
							AddThread(ptr->part->part);
						}
					}

					else
					{
						ptr->part->part = nullptr;
					}
				}
				break;
			}

			RemoveThread(index);
		}
	}
}

DWORD WINAPI Dummy(LPVOID arg)
{
	while (1)
	{
		Sleep(0);
	}

	return ERROR;
}

DWORD CALLBACK ProcessClient(LPVOID arg)
{
	_ClientInfo* Client_ptr = (_ClientInfo*)arg;

	char* result_str = nullptr;
	int result;

	bool endflag = false;
	while (1)
	{
		switch (Client_ptr->state)
		{
		case INTRO_SEND:
			if (!SendFunction(Intro, Client_ptr, INTRO))
			{
				Client_ptr->state = DISCONNECT;
				return -1;
			}

			else
			{
				Client_ptr->state = GVALUE_RECV;
			}

			break;
		case GVALUE_RECV:
			if (!RecvFunction(Client_ptr))
			{
				Client_ptr->state = DISCONNECT;
				return -1;
			}

			Client_ptr->state = RESULT_SEND;
			break;
		case RESULT_SEND:

			if (Client_ptr->part != nullptr && Client_ptr->part->gvalue!=NODATA)
			{
				result = who_win(Client_ptr->gvalue, Client_ptr->part->gvalue);

				switch (result)
				{
				case FIRST_WIN:
					result_str = win;
					break;
				case SECOND_WIN:
					result_str = lose;
					break;
				case NO_WIN:
					result_str = No_win;
					break;
				}

				if (!SendFunction(result_str, Client_ptr, RESULT))
				{
					Client_ptr->state = DISCONNECT;
					return -1;
				}

				Client_ptr->state = GAME_OVER;
				break;
			}
			break;

		case GAME_OVER:
			endflag = true;
			break;
		}

		if (endflag)
		{
			break;
		}
	}
	return 0;
}

bool SendFunction(char * _str, _ClientInfo* _player, PROTOCOL _protocol)
//���� : Ŭ���̾�Ʈ���� ���� ���ڿ�,Ŭ���̾�Ʈ ������ ���� _ClientInfo ����ü ������,� ������ ��Ŷ���� ������ ��������
{
	int str_size = strlen(_str);											//������ ���ڿ��� ���̸� �����մϴ�
	char * ptr = _player->packetbuf;										//������ Ŭ���̾�Ʈ�� ���ۿ� �����͸� �� �غ� �մϴ� 
	int size = sizeof(_protocol);					//��Ŷ ������ ��üũ�⸦ ����մϴ�
	//���������� ũ�� + ���ڿ��� ���̸� ���� int�� ������ ũ�� + ���ڿ� ����

	switch (_protocol)														//�������ݷ� ����ġ���� �����Ͽ�
	{
	case INTRO:
	case RESULT:
		size += +sizeof(str_size)+str_size;
		break;
	}

	memcpy(ptr, &size, sizeof(size));										//�������� ��ġ�� ����� ����
	ptr = ptr + sizeof(size);												//������ �������� ũ�⸸ŭ �̵�

	memcpy(ptr, &_protocol, sizeof(_protocol));								//�������� ��ġ�� ���������� ����
	ptr = ptr + sizeof(_protocol);											//������ ���������� ũ�⸸ŭ �̵�


	switch (_protocol)
	{
	case INTRO:
	case RESULT:
		memcpy(ptr, &str_size, sizeof(str_size));								//�������� ��ġ�� ���ڿ��� ũ�⸦ ����
		ptr = ptr + sizeof(str_size);											//������ ���ڿ��� ũ���� ũ�⸸ŭ �̵�

		memcpy(ptr, _str, str_size);											//�������� ��ġ�� ���ڿ��� ����
		ptr = ptr + str_size;													//������ ���ڿ��� ũ�⸸ŭ �̵�
		break;
	}

	if (send(_player->sock, _player->packetbuf, sizeof(size)+size, 0) == SOCKET_ERROR)	//������ ������� ��Ŷ�� �ش� Ŭ���̾�Ʈ���� �����ϴ�
	{
		char err_str[BUFSIZE];
		sprintf(err_str, "StringSend() Error Protocol Number : %d", _protocol);									//������ ��Ÿ�� �������� ���������� �������ڷ� �к��մϴ�
		err_display(err_str);															//������ err_display�� ����ϰ�
		return false;																	//������ �Լ��� �����Ű������ false�� ��ȯ�մϴ�																
	}

	return true;																		//���� send���� ����ε� �۾��� �����Ͽ��ٸ� ���⿡�� �Լ��� �����ϴ�
}

bool RecvFunction(_ClientInfo * _player)
{
	int size;
	int protocol;
	int retval;

	retval = recvn(_player->sock, (char*)&size, sizeof(size), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("g_value recv error()1");
		return false;
	}
	else if (retval == 0)
	{
		return 0;
	}

	retval = recvn(_player->sock, _player->packetbuf, size, 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("g_value recv error()2");
		return false;

	}
	else if (retval == 0)
	{
		return 0;
	}

	char * ptr = _player->packetbuf;
	memcpy(&protocol, ptr, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	switch (protocol)
	{
	case GVALUE:
		memcpy(&_player->gvalue, ptr, sizeof(_player->gvalue));
		break;
	}

	return true;
}

void AddThread(_ClientInfo * ptr)
{
	hThread[th_count] = CreateThread(NULL, 0, ProcessClient, ptr, 0, NULL);
	ptr->thread = hThread[th_count];
}

void RemoveThread(int index)
{
	for (int i = index ; i < th_count; i++)
	{
		hThread[i] = hThread[i + 1];
	}

	hThread[th_count] = NULL;
	th_count--;
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

int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0){
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

int who_win(int part1, int part2)
{
	if (part1 == part2)
		return NO_WIN;
	else if (part1 % 3 == (part2 + 1) % 3)
		return FIRST_WIN;
	else
		return SECOND_WIN;
}

_ClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr)
{
	_ClientInfo* ptr = new _ClientInfo;
	ZeroMemory(ptr, sizeof(_ClientInfo));
	ptr->sock = sock;
	memcpy(&(ptr->clientaddr), &clientaddr, sizeof(clientaddr));
	ptr->state = INTRO_SEND;
	ptr->gvalue = NODATA;

	ClientInfo[count] = ptr;
	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));
	count++;
	
	return ptr;
}

void RemoveClient(_ClientInfo* ptr)
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

bool MatchPartner(_ClientInfo* _ptr)
{
	for (int i = 0; i < count; i++)
	{
		if (_ptr != ClientInfo[i] && ClientInfo[i]->part == nullptr)
		{
			ClientInfo[i]->part = _ptr;
			_ptr->part = ClientInfo[i];
			return true;
		}
	}
	
	return false;
}

