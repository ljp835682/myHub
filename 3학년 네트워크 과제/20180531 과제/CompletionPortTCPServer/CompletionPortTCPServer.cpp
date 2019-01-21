#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>


#define SERVERPORT 9000
#define BUFSIZE 4096
#define IDSIZE 255
#define PWSIZE 255
#define NICKNAMESIZE 255

#define ID_ERROR_MSG "없는 아이디입니다\n"
#define PW_ERROR_MSG "패스워드가 틀렸습니다.\n"
#define LOGIN_SUCCESS_MSG "로그인에 성공했습니다.\n"
#define ID_EXIST_MSG "이미 있는 아이디 입니다.\n"
#define JOIN_SUCCESS_MSG "가입에 성공했습니다.\n"

enum RESULT { NODATA = -1, ID_EXIST = 1, ID_ERROR, PW_ERROR, JOIN_SUCCESS, LOGIN_SUCCESS };
enum PROTOCOL { JOIN_INFO, LOGIN_INFO, JOIN_RESULT, LOGIN_RESULT };
enum STATE {INIT_STATE ,MENU_SELECT_STATE = 1, JOIN_STATE, LOGIN_STATE, RESULT_SEND_STATE, DISCONNECTED_STATE };
enum { SOC_ERROR = 1, SOC_TRUE, SOC_FALSE };

//유저구조체
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

//클라이언트 구조체
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

// 작업자 스레드 함수
DWORD WINAPI WorkerThread(LPVOID arg);
// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);

//클라 구조체 생성 함수
_ClientInfo * AddClientInfo(SOCKET _sock, SOCKADDR_IN _addr)
{
	EnterCriticalSection(&cs);
	if (Client_Count >= WSA_MAXIMUM_WAIT_EVENTS)
	{
		return nullptr;
	}

	//초기화는 생성자에서
	_ClientInfo * ptr = new _ClientInfo(_sock, _addr);

	ptr->wsabuf.buf = ptr->recvbuf;
	ptr->wsabuf.len = BUFSIZE;
	ptr->state = MENU_SELECT_STATE;

	Client_List[Client_Count] = ptr;
	Client_Count++;
	LeaveCriticalSection(&cs);

	printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
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

//클라 삭제 함수
void RemoveClientInfo(_ClientInfo * _ptr)
{
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
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

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;

	// 입출력 완료 포트 생성
	// 새로운 핸들 생성
	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if(hcp == NULL) return 1;

	// CPU 개수 확인
	// 시스템의 정보를 인풋으로 넣는것
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// (CPU 개수 * 2)개의 작업자 스레드 생성
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

	// 데이터 통신에 사용할 변수
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
		printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", 
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 소켓과 입출력 완료 포트 연결
		CreateIoCompletionPort((HANDLE)client_sock, hcp, client_sock, 0);

		// 소켓 정보 구조체 할당
		_ClientInfo * ptr = AddClientInfo(client_sock, clientaddr);

		Recv(ptr);
	}

	// 윈속 종료
	WSACleanup();

	DeleteCriticalSection(&cs);

	return 0;
}

//소켓으로 해당되는 클라이언트를 찾아내는 함수
_ClientInfo * FindClientInfo(const SOCKET _sock)
{
	for (int i = 0; i < Client_Count; i++)
	{
		if (Client_List[i]->sock == _sock)
		{
			//찾을경우
			return Client_List[i];
		}
	}

	//못찾을경우
	return nullptr;
}

// 작업자 스레드 함수
DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;
	HANDLE hcp = (HANDLE)arg;
	
	while(1){
		// 비동기 입출력 완료 기다리기
		DWORD cbTransferred;
		SOCKET client_sock;
		_ClientInfo * ptr;
		WSAOVERLAPPED overlap;
		retval = GetQueuedCompletionStatus(hcp, &cbTransferred, (LPDWORD)&client_sock, (LPOVERLAPPED *)&overlap, INFINITE);

		ptr = FindClientInfo(client_sock);
		ptr->overlapped = overlap;

		// 클라이언트 정보 얻기
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (SOCKADDR *)&clientaddr, &addrlen);
		
		// 비동기 입출력 결과 확인
		if(retval == 0 || cbTransferred == 0)
		{
			//cbTransferred 이면 연결이 끊긴것, retval이 0이면 오류가 난것
			if(retval == 0)
			{
				DWORD temp1, temp2;
				WSAGetOverlappedResult(ptr->sock, &ptr->overlapped, &temp1, FALSE, &temp2);				

				err_display("WSAGetOverlappedResult()");
			}
			closesocket(ptr->sock);
			printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", 
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
			delete ptr;
			continue;
		}

		int retval = 0;

		//상태 확인
		switch (ptr->state)
		{
			//메뉴 선택 상태
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
				//회원가입
			case JOIN_INFO:
				UnPackPacket(ptr->recvbuf, ptr->temp_user->id, ptr->temp_user->pw, ptr->temp_user->nickname);
				ptr->state = JOIN_STATE;
				break;

				//로그인
			case LOGIN_INFO:
				UnPackPacket(ptr->recvbuf, ptr->temp_user->id, ptr->temp_user->pw);
				ptr->state = LOGIN_STATE;
				break;
			}

			ptr->InitializeRecvBuffer();
			break;

			//결과 전송 상태
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

			//상태 복구
			ptr->state = MENU_SELECT_STATE;

			//새로운 recv() 실행으로 끝나지 않도록 함
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

			//상태를 send 상태로
			ptr->state = RESULT_SEND_STATE;
			break;

			//로그인
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

			//상태를 send 상태로
			ptr->state = RESULT_SEND_STATE;
			break;
		}
	}

	return 0;
}

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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