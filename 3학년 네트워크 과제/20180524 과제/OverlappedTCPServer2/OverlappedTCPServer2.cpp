#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE 4096
#define IDSIZE 255
#define PWSIZE 255
#define NICKNAMESIZE 255
#define ERROR_DISCONNECTED -2
#define DISCONNECTED -1
#define SOC_TRUE 1
#define SOC_FALSE 0

#define ID_ERROR_MSG "���� ���̵��Դϴ�\n"
#define PW_ERROR_MSG "�н����尡 Ʋ�Ƚ��ϴ�.\n"
#define LOGIN_SUCCESS_MSG "�α��ο� �����߽��ϴ�.\n"
#define ID_EXIST_MSG "�̹� �ִ� ���̵� �Դϴ�.\n"
#define JOIN_SUCCESS_MSG "���Կ� �����߽��ϴ�.\n"

enum STATE
{
	INIT_STATE = -1,
	LOGIN_MENU_SELECT_STATE = 1,
	JOIN_STATE,
	LOGIN_STATE,
	LOGOUT_STATE,

	TENDER_MENU_SELECT_STATE,
	TENDER_REGISTER_MSG_SEND_STATE,
	TENDER_LIST_SEND_STATE,
	TENDER_PARTICIPATE_STATE,
	TENDER_COMPLETE_STATE,
	TENDER_ERROR_SEND_STATE,

	DISCONNECTED_STATE
};

enum RESULT
{
	NODATA = -1,
	ID_EXIST = 1,
	ID_ERROR,
	PW_ERROR,
	JOIN_SUCCESS,
	LOGIN_SUCCESS,
	TENDER_REGISTER_SUCCESS,

	TENDER_SUCCESS,
	TENDER_FAIL,
	TENDER_ONGOING,
	TENDER_COMPLETE,

	TENDER_CODE_ERROR
};

enum PROTOCOL
{
	JOIN_INFO,
	LOGIN_INFO,
	JOIN_RESULT,
	LOGIN_RESULT,
	LOGOUT,
	LOGOUT_RESULT,

	TENDER_REGISTER,
	TENDER_REGISTER_RESULT,
	REQ_TENDER_LIST_INFO,

	TENDER_LIST_INFO,
	TENDER_SELECT_INFO,
	TENDER_RESULT_INFO,
	TENDER_ERROR_INFO
};

struct _User_Info
{
	char	id[IDSIZE];
	char	pw[PWSIZE];
	char	nickname[NICKNAMESIZE];

	_User_Info()
	{
		ZeroMemory(this, sizeof(_User_Info));
	}

	void Init()
	{
		ZeroMemory(this, sizeof(_User_Info));
	}
};

struct _ClientInfo
{
	WSAOVERLAPPED					overlapped;

	SOCKET							sock;
	SOCKADDR_IN						addr;
	_User_Info*						userinfo;
	_User_Info*						temp_user;
	STATE							pre_state;
	STATE							state;

	int								recvbytes;
	int								comp_recvbytes;
	int								sendbytes;
	int								comp_sendbytes;

	bool							sizeRecvComple;
	bool							packetRecvComple;

	char							sendbuf[BUFSIZE];
	char							recvbuf[BUFSIZE];

	WSABUF							wsabuf;

	//�ܺο��� �ڽ��� �迭�� �ε��� ��ȣ�� �ڽŰ� ��ġ�Ǵ� �̺�Ʈ�� ������
	_ClientInfo()
	{
		temp_user = new _User_Info();

		pre_state = INIT_STATE;
		state = INIT_STATE;

		recvbytes = 0;
		comp_recvbytes = 0;
		sendbytes = 0;
		comp_sendbytes = 0;

		ZeroMemory(&overlapped, sizeof(overlapped));
		ZeroMemory(sendbuf, 0, BUFSIZE);
		ZeroMemory(recvbuf, 0, BUFSIZE);

		//recv�� ��������, �ʱ⿡ ���� ũ��� �������� ��Ŷ�� size�κ�
		wsabuf.buf = recvbuf;
		wsabuf.len = sizeof(int);

		sizeRecvComple = false;
		packetRecvComple = false;
	}

	~_ClientInfo()
	{
		delete temp_user;
	}

	void WSARecvBuf()
	{
		wsabuf.buf = recvbuf + comp_recvbytes;
		wsabuf.len = recvbytes;
	}

	void WSASendBuf()
	{
		wsabuf.buf = sendbuf + comp_sendbytes;
		wsabuf.len = sendbytes;
	}
};

_User_Info* Join_List[100];
int Join_Count = 0;


_ClientInfo * AddClient(SOCKET _sock, SOCKADDR_IN _clientaddr)
{
	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(_clientaddr.sin_addr),
		ntohs(_clientaddr.sin_port));

	//�ε��� ��ȣ�� �̺�Ʈ�� �ܺο��� ����ü �ȿ� �ִ´�
	_ClientInfo * ptr = new _ClientInfo();

	ptr->sock = _sock;
	memcpy(&ptr->addr, &_clientaddr, sizeof(SOCKADDR_IN));
	ptr->state = INIT_STATE;
	return ptr;
}

void RemoveClient(_ClientInfo * _ptr)
{
	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(_ptr->addr.sin_addr), ntohs(_ptr->addr.sin_port));
	closesocket(_ptr->sock);
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

SOCKET client_sock;
HANDLE hReadEvent, hWriteEvent;

// �񵿱� ����� ���۰� ó�� �Լ�
DWORD WINAPI WorkerThread(LPVOID arg);

// WorkerThread�� ������ ȣ��� ��ƾ
void CALLBACK CompletionRoutine(
	DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags
);
// ���� ��� �Լ�
void err_quit(char *msg);
void err_display(char *msg);
void err_display(int errcode);

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;

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

	// �̺�Ʈ ��ü ����
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if(hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(hWriteEvent == NULL) return 1;

	// ������ ����
	HANDLE hThread = CreateThread(NULL, 0, WorkerThread, NULL, 0, NULL);
	if(hThread == NULL) return 1;
	CloseHandle(hThread);

	while(1){
		WaitForSingleObject(hReadEvent, INFINITE);
		// accept()
		client_sock = accept(listen_sock, NULL, NULL);
		if(client_sock == INVALID_SOCKET){
			err_display("accept()");
			break;
		}
		SetEvent(hWriteEvent);
	}

	// ���� ����
	WSACleanup();
	return 0;
}

// �񵿱� ����� ���� �Լ�
//�������� ������ ������ ���ú길�� �����ִ� ��Ȱ
DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;

	while(1){
		while(1){
			// alertable wait
			//WaitForSingleObject"Ex" : ó������ cpu�ð��� �ȹ����� os �� �Ϸ� ��ƾ�� ȣ���ϸ鼭 ���⼭ �Ϸ��ƾ�� �����Ѵ�
			//�� �Ϸ� ��ƾ�� ������ �� �Լ��� �����Ѵ�

			//���ϵǴ� ���
			//1. hWriteEvent �̺�Ʈ�� ������ ���
			//2. �Ϸ��ƾ�� ������ �Ϸ��ƾ�� �����ٴ� �ǹ̷� �����°��
			DWORD result = WaitForSingleObjectEx(hWriteEvent, INFINITE, TRUE);
			
			//hWriteEvent �� �������
			if (result == WAIT_OBJECT_0) break;

			//�Ϸ��ƾ�� ������� ������ ���ļ� �ٽ� WaitForSingleObjectEx�� ����
			if (result != WAIT_IO_COMPLETION) return 1;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		
		getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// ���� ���� ����ü �Ҵ�� �ʱ�ȭ
	 	_ClientInfo * ptr = AddClient(client_sock, clientaddr);

		SetEvent(hReadEvent);
		ptr->recvbytes = sizeof(int);
		ptr->WSARecvBuf();

		// �񵿱� ����� ����
		DWORD recvbytes;
		DWORD flags = 0;

		//������ ���ú�
		retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes,
			&flags, &ptr->overlapped, CompletionRoutine);
		if(retval == SOCKET_ERROR)
		{
			if(WSAGetLastError() != WSA_IO_PENDING){
				err_display("WSARecv()");
				return 1;
			}
		}
	}
	return 0;
}

// �񵿱� ����� ó�� �Լ�(����� �Ϸ� ��ƾ)
void CALLBACK CompletionRoutine(
	DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	DWORD recvbytes;
	DWORD flags = 0;
	int retval;

	// Ŭ���̾�Ʈ ���� ���
	_ClientInfo * ptr = (_ClientInfo *)lpOverlapped;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR *)&clientaddr, &addrlen);

	// �񵿱� ����� ��� Ȯ��
	if(dwError != 0 || cbTransferred == 0){
		if(dwError != 0) err_display(dwError);
		closesocket(ptr->sock);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		RemoveClient(ptr);
		return;
	}

	//��Ŷ�� ����� �޴ºκ�
	if (!ptr->sizeRecvComple)
	{
		//���� ����� ���Ѵ�
		ptr->comp_recvbytes += cbTransferred;

		//���δ� �޾������
		if (ptr->comp_recvbytes == ptr->recvbytes)
		{
			//�ʱ�ȭ
			ptr->comp_recvbytes = 0;
			ptr->sizeRecvComple = true;

			//������ �޾ƾ��� ����� ���ۿ��� ����
			memcpy(&ptr->recvbytes, ptr->recvbuf, sizeof(int));	//ũ�⸦ ����
			ptr->WSARecvBuf();

			//���ο� ���ú�
			retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, CompletionRoutine);
			if (retval == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					err_display("WSARecv()");
					return;
				}
			}
		}

		//���δ� ���� ���Ұ��
		else
		{
			//recvbytes�� ũ�⸦ ������ä �����Ѵ�
			ptr->WSARecvBuf();

			//�̾ ���ú�
			retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, CompletionRoutine);
			if (retval == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					err_display("WSARecv()");
					return;
				}
			}
		}

		//���װ� ������ �� ��ƾ�� ����
		return;
	}

	//��Ŷ�� ������ �޴ºκ�
	if(!ptr->packetRecvComple && ptr->sizeRecvComple)
	{
		//���� ����� ���Ѵ�
		ptr->comp_recvbytes += cbTransferred;

		//���� ���� �Ǿ������
		if (ptr->comp_recvbytes == ptr->recvbytes)
		{
			//�ʱ�ȭ
			ptr->recvbytes = sizeof(int);
			ptr->comp_recvbytes = 0;

			//�ٹ����� �˸�
			ptr->packetRecvComple = true;
		}

		//�� ���� ���Ѱ��
		else
		{
			//recvbytes�� ũ�⸦ ������ä �����Ѵ�
			ptr->WSARecvBuf();

			//�̾ ���ú�
			retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, CompletionRoutine);
			if (retval == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					err_display("WSARecv()");
					return;
				}
			}

			//���� ����������Ŷ�� ����� �޴� if�� ���� �ٸ��� else������ ��ƾŻ��
			return;
		}
	}

	//��� ��Ŷ�� �޴°� ���������
	if (ptr->packetRecvComple)
	{
		//�ʱ�ȭ
		ptr->packetRecvComple = false;
		ptr->sizeRecvComple = false;

		//��ſ� �ʿ��� ������
		RESULT join_result = NODATA;
		RESULT login_result = NODATA;
		char msg[BUFSIZE];
		PROTOCOL protocol;
		int size;

		//�������� ��������
		GetProtocol(ptr->recvbuf, protocol);

		//�������ݿ� ���� �и�
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

		//���¿� ���� �и�
		switch (ptr->state)
		{
			//ȸ������
		case JOIN_STATE:
			for (int i = 0; i < Join_Count; i++)
			{
				if (!strcmp(Join_List[i]->id, ptr->temp_user->id))
				{
					join_result = ID_EXIST;
					strcpy(msg, ID_EXIST_MSG);
					break;
				}
			}

			if (join_result == NODATA)
			{
				_User_Info * user = new _User_Info();
				strcpy(user->id, ptr->temp_user->id);
				strcpy(user->pw, ptr->temp_user->pw);
				strcpy(user->nickname, ptr->temp_user->nickname);

				Join_List[Join_Count++] = user;
				ptr->temp_user->Init();

				join_result = JOIN_SUCCESS;
				strcpy(msg, JOIN_SUCCESS_MSG);
			}

			protocol = JOIN_RESULT;

			PackPacket(ptr->sendbuf, protocol, join_result, msg, size);

			retval = send(ptr->sock, ptr->sendbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("send()");
				ptr->state = DISCONNECTED_STATE;
				break;
			}

			ptr->state = LOGIN_MENU_SELECT_STATE;
			break;

			//�α���
		case LOGIN_STATE:

			for (int i = 0; i < Join_Count; i++)
			{
				if (!strcmp(Join_List[i]->id, ptr->temp_user->id))
				{
					if (!strcmp(Join_List[i]->pw, ptr->temp_user->pw))
					{
						login_result = LOGIN_SUCCESS;
						strcpy(msg, LOGIN_SUCCESS_MSG);
					}
					else
					{
						login_result = PW_ERROR;
						strcpy(msg, PW_ERROR_MSG);
					}
					break;
				}
			}

			if (login_result == NODATA)
			{
				login_result = ID_ERROR;
				strcpy(msg, ID_ERROR_MSG);
			}

			protocol = LOGIN_RESULT;

			PackPacket(ptr->sendbuf, protocol, login_result, msg, size);

			retval = send(ptr->sock, ptr->sendbuf, size, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				ptr->state = DISCONNECTED_STATE;
				break;
			}
			ptr->state = LOGIN_MENU_SELECT_STATE;
			break;
		}
	}

	ptr->WSARecvBuf();
	retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, CompletionRoutine);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING) 
		{
			err_display("WSARecv()");
			return;
		}
	}
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

// ���� �Լ� ���� ���
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[����] %s", (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}