#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>


#define SERVERPORT 9000
#define BUFSIZE 4096
#define IDSIZE 255
#define PWSIZE 255
#define NICKNAMESIZE 255

#define ID_ERROR_MSG "���� ���̵��Դϴ�\n"
#define PW_ERROR_MSG "�н����尡 Ʋ�Ƚ��ϴ�.\n"
#define LOGIN_SUCCESS_MSG "�α��ο� �����߽��ϴ�.\n"
#define ID_EXIST_MSG "�̹� �ִ� ���̵� �Դϴ�.\n"
#define JOIN_SUCCESS_MSG "���Կ� �����߽��ϴ�.\n"

enum RESULT { NODATA = -1, ID_EXIST = 1, ID_ERROR, PW_ERROR, JOIN_SUCCESS, LOGIN_SUCCESS };
enum PROTOCOL { JOIN_INFO, LOGIN_INFO, JOIN_RESULT, LOGIN_RESULT };
enum STATE {INIT_STATE ,MENU_SELECT_STATE = 1, JOIN_STATE, LOGIN_STATE, RESULT_SEND_STATE, DISCONNECTED_STATE };
enum { SOC_ERROR = 1, SOC_TRUE, SOC_FALSE };

//��������ü
struct _User_Info
{
	char id[IDSIZE];
	char pw[PWSIZE];
	char nickname[NICKNAMESIZE];

	_User_Info()
	{
		ZeroMemory(this, sizeof(_User_Info));
	}

	void Init()
	{
		ZeroMemory(this, sizeof(_User_Info));
	}
};

//Ŭ���̾�Ʈ ����ü
struct _ClientInfo
{
	WSAOVERLAPPED overlapped;

	SOCKET sock;
	SOCKADDR_IN addr;
	WSABUF wsabuf;
	STATE state;

	_User_Info *						userinfo;
	_User_Info *						temp_user;

	int recvbytes;
	int comp_recvbytes;
	int sendbytes;
	int comp_sendbytes;

	char recvbuf[BUFSIZE];
	char sendbuf[BUFSIZE];

	_ClientInfo(SOCKET _sock, SOCKADDR_IN _addr)
	{
		sock = _sock;
		addr = _addr;

		temp_user = new _User_Info();
		state = INIT_STATE;

		recvbytes = 0;
		comp_recvbytes = 0;
		sendbytes = 0;
		comp_sendbytes = 0;

		ZeroMemory(&wsabuf, sizeof(WSABUF));
		ZeroMemory(&overlapped, sizeof(WSAOVERLAPPED));
		ZeroMemory(recvbuf, sizeof(recvbuf));
		ZeroMemory(sendbuf, sizeof(sendbuf));
	}

	~_ClientInfo()
	{
		delete temp_user;
	}

	void InitializeRecvBuffer()
	{
		memcpy(recvbuf, recvbuf + recvbytes, comp_recvbytes);
		recvbytes = 0;
	}

	void InitializeOverlapped()
	{
		ZeroMemory(&overlapped, sizeof(WSAOVERLAPPED));
	}
};

_User_Info * Join_List[100];
int Join_Count = 0;

_ClientInfo * Client_List[100];
int Client_Count = 0;

CRITICAL_SECTION cs;

SOCKET client_sock;

// �۾��� ������ �Լ�
DWORD WINAPI WorkerThread(LPVOID arg);
// ���� ��� �Լ�
void err_quit(char *msg);
void err_display(char *msg);

//Ŭ�� ����ü ���� �Լ�
_ClientInfo * AddClientInfo(SOCKET _sock, SOCKADDR_IN _addr)
{
	EnterCriticalSection(&cs);
	if (Client_Count >= WSA_MAXIMUM_WAIT_EVENTS)
	{
		return nullptr;
	}

	//�ʱ�ȭ�� �����ڿ���
	_ClientInfo * ptr = new _ClientInfo(_sock, _addr);

	ptr->wsabuf.buf = ptr->recvbuf;
	ptr->wsabuf.len = BUFSIZE;
	ptr->state = MENU_SELECT_STATE;

	Client_List[Client_Count] = ptr;
	Client_Count++;
	LeaveCriticalSection(&cs);

	printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(ptr->addr.sin_addr), ntohs(ptr->addr.sin_port));

	return ptr;
}

void GetProtocol(char* _ptr, PROTOCOL& _protocol)
{
	memcpy(&_protocol, _ptr, sizeof(PROTOCOL));
}

void PackPacket(char* _buf, PROTOCOL _protocol, RESULT _result, char* _str1, int& _size)
{
	char* ptr = _buf;
	int strsize1 = strlen(_str1);
	_size = 0;

	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);

	memcpy(ptr, &_result, sizeof(_result));
	ptr = ptr + sizeof(_result);
	_size = _size + sizeof(_result);

	memcpy(ptr, &strsize1, sizeof(strsize1));
	ptr = ptr + sizeof(strsize1);
	_size = _size + sizeof(strsize1);

	memcpy(ptr, _str1, strsize1);
	ptr = ptr + strsize1;
	_size = _size + strsize1;

	ptr = _buf;
	memcpy(ptr, &_size, sizeof(_size));

	_size = _size + sizeof(_size);
}

void UnPackPacket(char* _buf, char* _str1, char* _str2, char* _str3)
{
	int str1size, str2size, str3size;

	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&str1size, ptr, sizeof(str1size));
	ptr = ptr + sizeof(str1size);

	memcpy(_str1, ptr, str1size);
	ptr = ptr + str1size;

	memcpy(&str2size, ptr, sizeof(str2size));
	ptr = ptr + sizeof(str2size);

	memcpy(_str2, ptr, str2size);
	ptr = ptr + str2size;

	memcpy(&str3size, ptr, sizeof(str3size));
	ptr = ptr + sizeof(str3size);

	memcpy(_str3, ptr, str3size);
	ptr = ptr + str3size;
}

void UnPackPacket(char* _buf, char* _str1, char* _str2)
{
	int str1size, str2size;

	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&str1size, ptr, sizeof(str1size));
	ptr = ptr + sizeof(str1size);

	memcpy(_str1, ptr, str1size);
	ptr = ptr + str1size;

	memcpy(&str2size, ptr, sizeof(str2size));
	ptr = ptr + sizeof(str2size);

	memcpy(_str2, ptr, str2size);
	ptr = ptr + str2size;
}

//Ŭ�� ���� �Լ�
void RemoveClientInfo(_ClientInfo * _ptr)
{
	printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(_ptr->addr.sin_addr), ntohs(_ptr->addr.sin_port));

	EnterCriticalSection(&cs);
	for (int i = 0; i < Client_Count; i++)
	{
		if (Client_List[i] == _ptr)
		{
			closesocket(Client_List[i]->sock);

			delete Client_List[i];

			for (int j = i; j < Client_Count - 1; j++)
			{
				Client_List[j] = Client_List[j + 1];
			}

			break;
		}
	}

	Client_Count--;
	LeaveCriticalSection(&cs);

}

bool Recv(_ClientInfo * _ptr)
{
	int retval;
	DWORD recvbytes;
	DWORD flags = 0;

	ZeroMemory(&_ptr->overlapped, sizeof(_ptr->overlapped));
	_ptr->wsabuf.buf = _ptr->recvbuf + _ptr->comp_recvbytes;
	_ptr->wsabuf.len = BUFSIZE - _ptr->comp_recvbytes;

	retval = WSARecv(_ptr->sock, &_ptr->wsabuf, 1, &recvbytes,
		&flags, &_ptr->overlapped, NULL);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			err_display("WSARecv()");
			RemoveClientInfo(_ptr);
			return false;
		}
	}

	return true;
}

bool Send(_ClientInfo * _ptr, int _size)
{
	int retval;
	DWORD sendbytes;
	DWORD flags;

	ZeroMemory(&_ptr->overlapped, sizeof(_ptr->overlapped));

	if (_ptr->sendbytes == 0)
	{
		_ptr->sendbytes = _size;
	}

	_ptr->wsabuf.buf = _ptr->sendbuf + _ptr->comp_sendbytes;
	_ptr->wsabuf.len = _ptr->sendbytes - _ptr->comp_sendbytes;

	retval = WSASend(_ptr->sock, &_ptr->wsabuf, 1, &sendbytes,
		0, &_ptr->overlapped, NULL);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			err_display("WSASend()");
			RemoveClientInfo(_ptr);
			return false;
		}
	}

	return true;
}

int OverlapSend(_ClientInfo * _ptr, int _sendbyte)
{
	_ptr->comp_sendbytes += _sendbyte;
	if (_ptr->comp_sendbytes == _ptr->sendbytes)
	{
		_ptr->comp_sendbytes = 0;
		_ptr->sendbytes = 0;

		return SOC_TRUE;
	}
	if (!Send(_ptr, _ptr->sendbytes))
	{
		return SOC_ERROR;
	}

	return SOC_FALSE;
}

int OverlapRecv(_ClientInfo * _ptr, int _recvbyte)
{

	if (_ptr->recvbytes < sizeof(int))
	{
		_ptr->comp_recvbytes += _recvbyte;

		if (_ptr->comp_recvbytes >= sizeof(int))
		{
			memcpy(&_ptr->recvbytes, _ptr->recvbuf, sizeof(int));
			_ptr->comp_recvbytes = _ptr->comp_recvbytes - sizeof(int);
			memcpy(_ptr->recvbuf, _ptr->recvbuf + sizeof(int), _ptr->comp_recvbytes);

			if (_ptr->comp_recvbytes >= _ptr->recvbytes)
			{
				_ptr->comp_recvbytes -= _ptr->recvbytes;
				return SOC_TRUE;
			}
			else
			{
				if (!Recv(_ptr))
				{
					return SOC_ERROR;
				}
			}

		}
		else
		{
			if (!Recv(_ptr))
			{
				return SOC_ERROR;
			}
		}
	}

	_ptr->comp_recvbytes += _recvbyte;

	if (_ptr->comp_recvbytes >= _ptr->recvbytes)
	{
		_ptr->comp_recvbytes -= _ptr->recvbytes;
		return SOC_TRUE;
	}
	else
	{
		if (!Recv(_ptr))
		{
			return SOC_ERROR;
		}

		return SOC_FALSE;
	}
}


int main(int argc, char *argv[])
{
	int retval;

	InitializeCriticalSection(&cs);

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;

	// ����� �Ϸ� ��Ʈ ����
	// ���ο� �ڵ� ����
	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if(hcp == NULL) return 1;

	// CPU ���� Ȯ��
	// �ý����� ������ ��ǲ���� �ִ°�
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// (CPU ���� * 2)���� �۾��� ������ ����
	HANDLE hThread;
	for(int i=0; i<(int)si.dwNumberOfProcessors*2; i++){
		hThread = CreateThread(NULL, 0, WorkerThread, hcp, 0, NULL);
		if(hThread == NULL) return 1;
		CloseHandle(hThread);
	}

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");
	
	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	DWORD recvbytes, flags;

	while(1){
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET){
			err_display("accept()");
			break;
		}
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", 
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// ���ϰ� ����� �Ϸ� ��Ʈ ����
		CreateIoCompletionPort((HANDLE)client_sock, hcp, client_sock, 0);

		// ���� ���� ����ü �Ҵ�
		_ClientInfo * ptr = AddClientInfo(client_sock, clientaddr);

		Recv(ptr);
	}

	// ���� ����
	WSACleanup();

	DeleteCriticalSection(&cs);

	return 0;
}

//�������� �ش�Ǵ� Ŭ���̾�Ʈ�� ã�Ƴ��� �Լ�
_ClientInfo * FindClientInfo(const SOCKET _sock)
{
	for (int i = 0; i < Client_Count; i++)
	{
		if (Client_List[i]->sock == _sock)
		{
			//ã�����
			return Client_List[i];
		}
	}

	//��ã�����
	return nullptr;
}

// �۾��� ������ �Լ�
DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;
	HANDLE hcp = (HANDLE)arg;
	
	while(1){
		// �񵿱� ����� �Ϸ� ��ٸ���
		DWORD cbTransferred;
		SOCKET client_sock;
		_ClientInfo * ptr;
		WSAOVERLAPPED overlap;
		retval = GetQueuedCompletionStatus(hcp, &cbTransferred, (LPDWORD)&client_sock, (LPOVERLAPPED *)&overlap, INFINITE);

		ptr = FindClientInfo(client_sock);
		ptr->overlapped = overlap;

		// Ŭ���̾�Ʈ ���� ���
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (SOCKADDR *)&clientaddr, &addrlen);
		
		// �񵿱� ����� ��� Ȯ��
		if(retval == 0 || cbTransferred == 0)
		{
			//cbTransferred �̸� ������ �����, retval�� 0�̸� ������ ����
			if(retval == 0)
			{
				DWORD temp1, temp2;
				WSAGetOverlappedResult(ptr->sock, &ptr->overlapped, &temp1, FALSE, &temp2);				

				err_display("WSAGetOverlappedResult()");
			}
			closesocket(ptr->sock);
			printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", 
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
			delete ptr;
			continue;
		}

		int retval = 0;

		//���� Ȯ��
		switch (ptr->state)
		{
			//�޴� ���� ����
		case MENU_SELECT_STATE:
			//recv()
			retval = OverlapRecv(ptr, cbTransferred);
			switch (retval)
			{
			case SOC_ERROR:
				return 0;
			case SOC_FALSE:
				return 0;
			case SOC_TRUE:
				break;
			}

			PROTOCOL protocol;
			GetProtocol(ptr->recvbuf, protocol);

			switch (protocol)
			{
				//ȸ������
			case JOIN_INFO:
				UnPackPacket(ptr->recvbuf, ptr->temp_user->id, ptr->temp_user->pw, ptr->temp_user->nickname);
				ptr->state = JOIN_STATE;
				break;

				//�α���
			case LOGIN_INFO:
				UnPackPacket(ptr->recvbuf, ptr->temp_user->id, ptr->temp_user->pw);
				ptr->state = LOGIN_STATE;
				break;
			}

			ptr->InitializeRecvBuffer();
			break;

			//��� ���� ����
		case RESULT_SEND_STATE:

			//send()
			retval = OverlapSend(ptr, cbTransferred);
			switch (retval)
			{
			case SOC_ERROR:
				return 0;
			case SOC_FALSE:
				return 0;
			case SOC_TRUE:
				break;
			}

			//���� ����
			ptr->state = MENU_SELECT_STATE;

			//���ο� recv() �������� ������ �ʵ��� ��
			if (!Recv(ptr))
			{
				return 0;
			}

			break;
		}

		RESULT result = NODATA;

		char msg[BUFSIZE];
		PROTOCOL protocol;
		int size;

		switch (ptr->state)
		{
		case JOIN_STATE:
			for (int i = 0; i < Join_Count; i++)
			{
				if (!strcmp(Join_List[i]->id, ptr->temp_user->id))
				{
					result = ID_EXIST;
					strcpy(msg, ID_EXIST_MSG);
					break;
				}
			}

			if (result == NODATA)
			{
				_User_Info * user = new _User_Info();
				strcpy(user->id, ptr->temp_user->id);
				strcpy(user->pw, ptr->temp_user->pw);
				strcpy(user->nickname, ptr->temp_user->nickname);

				Join_List[Join_Count++] = user;
				ptr->temp_user->Init();

				result = JOIN_SUCCESS;
				strcpy(msg, JOIN_SUCCESS_MSG);
			}

			protocol = JOIN_RESULT;

			PackPacket(ptr->sendbuf, protocol, result, msg, size);

			if (!Send(ptr, size))
			{
				return 0;
			}

			//���¸� send ���·�
			ptr->state = RESULT_SEND_STATE;
			break;

			//�α���
		case LOGIN_STATE:

			for (int i = 0; i < Join_Count; i++)
			{
				if (!strcmp(Join_List[i]->id, ptr->temp_user->id))
				{
					if (!strcmp(Join_List[i]->pw, ptr->temp_user->pw))
					{
						result = LOGIN_SUCCESS;
						strcpy(msg, LOGIN_SUCCESS_MSG);
					}
					else
					{
						result = PW_ERROR;
						strcpy(msg, PW_ERROR_MSG);
					}
					break;
				}
			}

			if (result == NODATA)
			{
				result = ID_ERROR;
				strcpy(msg, ID_ERROR_MSG);
			}

			protocol = LOGIN_RESULT;

			PackPacket(ptr->sendbuf, protocol, result, msg, size);

			if (!Send(ptr, size))
			{
				return 0;
			}

			//���¸� send ���·�
			ptr->state = RESULT_SEND_STATE;
			break;
		}
	}

	return 0;
}

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