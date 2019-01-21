/*
* echo_selserv_win.c
* Written by SW. YOON
*/

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#define BUFSIZE 512
#define PLAYERCOUNT 2
#define MAXUSER   10
#define NODATA   -1

enum { NO_WIN, FIRST_WIN, SECOND_WIN = -1 };
enum STATE{ INTRO_SEND, GVALUE_RECV, RESULT_SEND, GAME_OVER, CONNECT_END };
enum PROTOCOL{ INTRO, GVALUE, RESULT };

struct _ClientInfo
{
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	STATE state;
	int	gvalue;
	char  packetbuf[BUFSIZE];
	_ClientInfo* part;
	HANDLE hthread;
	HANDLE eventHandle;							//���� �Է¹ް� �����°� ����
	HANDLE catchHandle;							//�߰��� �������� ����� ���� ����
	HANDLE endhandle;							//������ ���� ��ȯ�ϴ� ��Ȳ�� ����
};


int who_win(int part1, int part2);
void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);

_ClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr);
void RemoveClient(_ClientInfo* ptr);

bool MatchPartner(_ClientInfo* _ptr);

DWORD CALLBACK ProcessClient(LPVOID);
DWORD CALLBACK WaitClient(LPVOID);

void AddThread(LPTHREAD_START_ROUTINE process, LPVOID _ptr);
void RemoveThread(int index);

void PackPacket(char*, PROTOCOL, char*, int&);
void GetProtocol(char*, PROTOCOL&);
void UnPackPactket(char*, int&);


_ClientInfo* ClientInfo[MAXUSER];
HANDLE hThread[MAXUSER];

int count = 0;
int threadcount = 0;

char No_win[] = "�����ϴ�.!!";
char win[] = "�̰���ϴ�!!";
char lose[] = "�����ϴ�!!!!";
char Intro[] = "����:0, ����:1, ��:2 �� �Է��ϼ���:";

CRITICAL_SECTION cs;
int main(int argc, char **argv)
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;
	InitializeCriticalSection(&cs);
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

	hThread[threadcount] = CreateEvent(NULL, FALSE, FALSE, NULL);					//�ڵ� �̺�Ʈ ����
													
	if (hThread[threadcount] == NULL)
	{
		return 1;
	}

	else
	{
		threadcount++;
	}

	CreateThread(nullptr, 0, WaitClient, nullptr, 0, nullptr);

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

		if (MatchPartner(ptr))
		{
			if (ptr->part->hthread == nullptr)
			{
				AddThread(ProcessClient, ptr->part);			
				SetEvent(hThread[0]);							//Ŭ���̾�Ʈ�� ���� �ٲ�� �˸������� ��ȣ��ȯ
			}
		}

		AddThread(ProcessClient, ptr);
		SetEvent(hThread[0]);									//Ŭ���̾�Ʈ�� ���� �ٲ�� �˸������� ��ȣ��ȯ
	}

	CloseHandle(hThread[0]);									//0��  �̺�Ʈ�� �̺�Ʈ�� �����ϴ� �κ� �̺κ��� ���� �ü�����
	closesocket(listen_sock);
	DeleteCriticalSection(&cs);
	WSACleanup();
	return 0;
}

void AddThread(LPTHREAD_START_ROUTINE process, LPVOID _ptr)
{
	EnterCriticalSection(&cs);
	_ClientInfo* ptr = (_ClientInfo*)_ptr;
	ptr->hthread = CreateThread(NULL, 0, process, ptr, 0, nullptr);
	hThread[threadcount++] = ptr->hthread;
	LeaveCriticalSection(&cs);
}

void RemoveThread(int index)
{
	EnterCriticalSection(&cs);
	int i;
	for (i = index; i < threadcount - 1; i++)
	{
		hThread[i] = hThread[i + 1];
	}

	hThread[i] = nullptr;
	threadcount--;
	LeaveCriticalSection(&cs);
}

DWORD CALLBACK ProcessClient(LPVOID  _ptr)
{
	_ClientInfo* Client_ptr = (_ClientInfo*)_ptr;

	char* result_str = nullptr;
	int size;
	int strsize;
	PROTOCOL protocol;
	int result;
	int retval;

	//char* ptr;
	//bool endlflag = false;

	while (1)
	{
		switch (Client_ptr->state)
		{
		case INTRO_SEND:	
			PackPacket(Client_ptr->packetbuf, INTRO, Intro, size);	

			if (send(Client_ptr->sock, Client_ptr->packetbuf, size, 0) == SOCKET_ERROR)
			{
				err_display("IntroSend()");
				Client_ptr->state = CONNECT_END;
				return -1;
			}
			else
			{
				Client_ptr->state = GVALUE_RECV;
			}

		case GVALUE_RECV:	
			retval = recvn(Client_ptr->sock, (char*)&size, sizeof(size), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("gvalue recv error()");
				Client_ptr->state = CONNECT_END;
				break;

			}
			else if (retval == 0)
			{
				Client_ptr->state = CONNECT_END;
				break;
			}

			retval = recvn(Client_ptr->sock, Client_ptr->packetbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("gvalue recv error()");
				Client_ptr->state = CONNECT_END;
				break;

			}
			else if (retval == 0)
			{
				Client_ptr->state = CONNECT_END;
				break;
			}

			GetProtocol(Client_ptr->packetbuf, protocol);

			if (protocol == GVALUE)
			{
				int value;
				UnPackPactket(Client_ptr->packetbuf, value);
				Client_ptr->gvalue = value;	
				SetEvent(Client_ptr->eventHandle);								//�ڽ��� �Է��� �Ϸ�Ǿ����Ƿ� ��ȣ��ȯ 				
			}

			Client_ptr->state = RESULT_SEND;

		case RESULT_SEND:
			if (Client_ptr->part != nullptr)
			{
				if (Client_ptr->part->gvalue != NODATA)
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

					PackPacket(Client_ptr->packetbuf, RESULT, result_str, size);

					if (send(Client_ptr->sock, Client_ptr->packetbuf, size, 0) == SOCKET_ERROR)
					{
						err_display("IntroSend()");
					}

					Client_ptr->state = GAME_OVER;
				}
				else
				{
					int retval = WaitForSingleObject(Client_ptr->part->eventHandle, INFINITE);		//��Ʈ�ʰ� ���������� ���
					if (retval != WAIT_OBJECT_0)
					{
						return 0;																
					}

					if (Client_ptr->part->state == CONNECT_END)										//���� ��Ʈ�ʰ� ��������̶��
					{
						SetEvent(Client_ptr->catchHandle);											//�ڽ��� ĳġ �̺�Ʈ�� Ű��
						return 0;																	//����
					}
				}

				if (Client_ptr->state == GAME_OVER)													//���������
				{	
					SetEvent(Client_ptr->endhandle);												//�ڽ��� ���� �̺�Ʈ�� Ű��

					int retval = WaitForSingleObject(Client_ptr->part->endhandle, INFINITE);		//��Ʈ�ʰ� ��������Ǵ°��� ��ٸ�
					
					if (retval != WAIT_OBJECT_0)
					{
						return -1;
					}

					return 0;																		//����
				}				

			}
			else
			{
				return -1;
			}
		}

		if (Client_ptr->state == CONNECT_END && Client_ptr->part != nullptr)				//Ŭ�� ��������� ��Ʈ�ʰ� �ִٸ�
		{
			SetEvent(Client_ptr->eventHandle);												//��Ʈ������ �ڽ��� ����Ȱ��� �˸�
			int retval;	
			retval = WaitForSingleObject(Client_ptr->part->catchHandle, INFINITE);			//��Ʈ���� ĳġ �̺�Ʈ�� ��ٸ�
			return 0;																		//����
		}

		else if (Client_ptr->state == CONNECT_END && Client_ptr->part == nullptr)			//Ŭ�� ���� ����� ��Ʈ�ʰ� ���ٸ�
		{
			return -1;
		}
	}

	return 0;
}

void PackPacket(char* _buf, PROTOCOL _protocol, char* _str, int& _size)
{
	char* ptr = _buf;
	int strsize = strlen(_str);
	_size = sizeof(_protocol)+sizeof(strsize)+strsize;

	memcpy(ptr, &_size, sizeof(_size));
	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);

	memcpy(ptr, &strsize, sizeof(strsize));
	ptr = ptr + sizeof(strsize);

	memcpy(ptr, _str, strsize);	

	_size = _size + sizeof(_size);
}

void GetProtocol(char* _ptr, PROTOCOL& _protocol)
{	
	memcpy(&_protocol, _ptr, sizeof(PROTOCOL));	

}

void UnPackPactket(char* _buf, int& _value)
{
	memcpy(&_value, _buf+sizeof(PROTOCOL), sizeof(_value));
}

DWORD CALLBACK WaitClient(LPVOID _ptr)
{
	while (1)
	{
		int index = WaitForMultipleObjects(threadcount, hThread, false, INFINITE);
		index -= WAIT_OBJECT_0;
		DWORD threadid;

		if (index == 0)				//���� 0�ϰ��
		{
			continue;				//hThread[0]�� ���� �̺�Ʈ�� �ڵ��̺�Ʈ�̹Ƿ� ������ ����
		}

		for (int i = 0; i < count; i++)
		{
			HANDLE hthread = nullptr;

			if (ClientInfo[i]->hthread == hThread[index])
			{
				EnterCriticalSection(&cs);

				ClientInfo[i]->hthread = nullptr;

				switch (ClientInfo[i]->state)
				{
				case GAME_OVER:
					RemoveClient(ClientInfo[i]);
					break;
				case RESULT_SEND:

					break;
				case CONNECT_END:
					if (ClientInfo[i]->part != nullptr)
					{
						if (MatchPartner(ClientInfo[i]->part))
						{
							if (ClientInfo[i]->part->hthread == nullptr)
							{
								AddThread(ProcessClient, ClientInfo[i]->part);
							}

							if (ClientInfo[i]->part->part->hthread == nullptr)
							{
								AddThread(ProcessClient, ClientInfo[i]->part->part);
							}

						}
						else
						{
							ClientInfo[i]->part->part = nullptr;
						}
					}
					RemoveClient(ClientInfo[i]);

					break;
				}

				RemoveThread(index);
				LeaveCriticalSection(&cs);
				break;
			}
		}
	}
}

bool MatchPartner(_ClientInfo* _ptr)
{
	EnterCriticalSection(&cs);
	for (int i = 0; i < count; i++)
	{
		if (_ptr != ClientInfo[i] && ClientInfo[i]->part == nullptr)
		{
			ClientInfo[i]->part = _ptr;
			_ptr->part = ClientInfo[i];
			LeaveCriticalSection(&cs);
			return true;
		}
	}
	LeaveCriticalSection(&cs);
	return false;
}

void err_quit(char *msg)
{
	LPVOID lpMsgbuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgbuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgbuf, msg, MB_ICONERROR);
	LocalFree(lpMsgbuf);
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
	LPVOID lpMsgbuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgbuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgbuf);
	LocalFree(lpMsgbuf);
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
	EnterCriticalSection(&cs);
	_ClientInfo* ptr = new _ClientInfo;
	ZeroMemory(ptr, sizeof(_ClientInfo));
	ptr->sock = sock;
	memcpy(&(ptr->clientaddr), &clientaddr, sizeof(clientaddr));
	ptr->state = INTRO_SEND;
	ptr->gvalue = NODATA;
	ptr->eventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);					//���� �̺�Ʈ ����
	ptr->catchHandle = CreateEvent(NULL, TRUE, FALSE, NULL);					//���� �̺�Ʈ ����
	ptr->endhandle = CreateEvent(NULL, TRUE, FALSE, NULL);						//���� �̺�Ʈ ����

	ClientInfo[count] = ptr;
	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));
	count++;
	LeaveCriticalSection(&cs);
	return ptr;
}

void RemoveClient(_ClientInfo* ptr)
{
	closesocket(ptr->sock);

	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(ptr->clientaddr.sin_addr),
		ntohs(ptr->clientaddr.sin_port));

	EnterCriticalSection(&cs);				

	CloseHandle(ptr->eventHandle);							//�̺�Ʈ  �̺�Ʈ ����

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
	LeaveCriticalSection(&cs);
}