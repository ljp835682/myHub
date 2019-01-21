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

#define ID_ERROR_MSG "없는 아이디입니다\n"
#define PW_ERROR_MSG "패스워드가 틀렸습니다.\n"
#define LOGIN_SUCCESS_MSG "로그인에 성공했습니다.\n"
#define ID_EXIST_MSG "이미 있는 아이디 입니다.\n"
#define JOIN_SUCCESS_MSG "가입에 성공했습니다.\n"



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
	SOCKET							sock;
	SOCKADDR_IN						addr;
	_User_Info*						userinfo;
	_User_Info*						temp_user;
	STATE							pre_state;
	STATE							state;

	WSAOVERLAPPED					overlapped;

	int								recvbytes;
	int								comp_recvbytes;
	int								sendbytes;
	int								comp_sendbytes;

	int								index;

	bool							sizeRecvComple;
	bool							packetRecvComple;

	char							sendbuf[BUFSIZE];
	char							recvbuf[BUFSIZE];

	WSABUF							wsabuf;

	//외부에서 자신의 배열속 인덱스 번호와 자신과 매치되는 이벤트를 가져옴
	_ClientInfo(WSAEVENT _hEvent,int _index)
	{
		temp_user = new _User_Info();

		pre_state = INIT_STATE;
		state = INIT_STATE;

		recvbytes = 0;
		comp_recvbytes = 0;
		sendbytes = 0;
		comp_sendbytes = 0;

		index = _index;						//인덱스 대입

		ZeroMemory(&overlapped, sizeof(overlapped));
		overlapped.hEvent = _hEvent;		//이벤트 대입

		ZeroMemory(sendbuf, 0, BUFSIZE);
		ZeroMemory(recvbuf, 0, BUFSIZE);

		//recv를 기준으로, 초기에 받을 크기는 가변길이 패킷의 size부분
		wsabuf.buf = recvbuf;
		wsabuf.len = sizeof(int);

		sizeRecvComple = false;
		packetRecvComple = false;
	}

	~_ClientInfo()
	{
		delete temp_user;
	}
};


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

WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
_ClientInfo* User_List[WSA_MAXIMUM_WAIT_EVENTS];
int nTotalSockets = 0;

_User_Info* Join_List[100];
int Join_Count = 0;

CRITICAL_SECTION cs;

// 비동기 입출력 처리 함수
DWORD WINAPI WorkerThread(LPVOID arg);

// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);

_ClientInfo* AddClient(SOCKET _sock, SOCKADDR_IN _clientaddr);
void RemoveClient(int _index);

int main(int argc, char *argv[])
{
	int retval;
	InitializeCriticalSection(&cs);

	// 윈속 초기화
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

	// 더미(dummy) 이벤트 객체 생성
	WSAEVENT hEvent = WSACreateEvent();
	if(hEvent == WSA_INVALID_EVENT)
		err_quit("WSACreateEvent()");
	EventArray[nTotalSockets++] = hEvent;

	// 스레드 생성
	HANDLE hThread = CreateThread(NULL, 0, WorkerThread, NULL, 0, NULL);
	if(hThread == NULL) return 1;
	CloseHandle(hThread);

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
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		//새로운 클라이언트 생성
		_ClientInfo * ptr = AddClient(client_sock, clientaddr);

		//새로운 클라이언트 생성 실패
		if(ptr == nullptr)
		{
			//만들어진 소켓삭제
			closesocket(client_sock);

			printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
			continue;
		}

		//새로운 클라이언트 생성 성공
		else
		{
			//상태 변경
			ptr->state = LOGIN_MENU_SELECT_STATE;
		}

		// 비동기 입출력 시작
		flags = 0;
		retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
		if(retval == SOCKET_ERROR)
		{
			if(WSAGetLastError() != WSA_IO_PENDING)
			{
				err_display("WSARecv()");
				RemoveClient(nTotalSockets-1);
				continue;
			}
		}

		// 소켓의 개수(nTotalSockets) 변화를 알림
		WSASetEvent(EventArray[0]);
	}

	// 윈속 종료
	WSACleanup();
	DeleteCriticalSection(&cs);
	return 0;
}

// 비동기 입출력 처리 함수
DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;

	while(1)
	{
		// 이벤트 객체 관찰
		DWORD index = WSAWaitForMultipleEvents(nTotalSockets, EventArray, FALSE, WSA_INFINITE, FALSE);
		if(index == WSA_WAIT_FAILED) continue;
		index -= WSA_WAIT_EVENT_0;
		
		WSAResetEvent(EventArray[index]);
		if(index == 0) continue;

		// 클라이언트 정보 얻기
		_ClientInfo * ptr = User_List[index];
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (SOCKADDR *)&clientaddr, &addrlen);

		// 비동기 입출력 결과 확인
		DWORD cbTransferred, flags;
		retval = WSAGetOverlappedResult(ptr->sock, &ptr->overlapped, &cbTransferred, FALSE, &flags);
		
		if(retval == FALSE || cbTransferred == 0)
		{
			RemoveClient(index);
			printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
			continue;
		}

		//가변길이패킷의 크기부분을 받게 하기전 전처리
		if (ptr->recvbytes == 0)
		{
			ptr->recvbytes = sizeof(int);
		}

		// 데이터 전송량 갱신
		if(!ptr->sizeRecvComple)
		{
			ptr->comp_recvbytes += cbTransferred;					//전송량을 comp_recvbytes 에 합산

			//전부 전송 되었을경우
			if (ptr->comp_recvbytes == ptr->recvbytes)				
			{
				memcpy(&ptr->recvbytes, ptr->recvbuf, sizeof(int));	//크기를 복사
				ptr->comp_recvbytes = 0;							
				ptr->sizeRecvComple = true;

				//가변길이패킷의 패킷부분을 받게 하는 부분의 전처리
				ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
				ptr->overlapped.hEvent = EventArray[index];
				ptr->wsabuf.buf = ptr->recvbuf;
				ptr->wsabuf.len = ptr->recvbytes;					

				DWORD recvbytes;

				//가변길이패킷의 패킷부분을 받는 부분
				retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
				if (retval == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						err_display("WSARecv()");
					}
				}

				continue;
			}

			//다 받지 못하였을경우 다시 받기
			else
			{
				ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
				ptr->overlapped.hEvent = EventArray[index];
				ptr->wsabuf.buf = ptr->recvbuf + ptr->comp_recvbytes;
				ptr->wsabuf.len = ptr->recvbytes;

				DWORD recvbytes;

				flags = 0;
				retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
				if (retval == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						err_display("WSARecv()");
					}
				}

				continue;
			}
		}

		//가변길이의 사이즈를받았지만 가변길이의 패킷을 다 받지 못하였을경우
		if(!ptr->packetRecvComple && ptr->sizeRecvComple)
		{
			ptr->comp_recvbytes += cbTransferred;				//전송량을 comp_recvbytes 에 합산

			//전부 전송 되엇을경우
			if (ptr->comp_recvbytes == ptr->recvbytes)			
			{
				//전부 초기화
				ptr->recvbytes = 0;
				ptr->comp_recvbytes = 0;
				ptr->packetRecvComple = true;
			}

			//다 받지 못하였을경우 다시 받기
			else
			{
				ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
				ptr->overlapped.hEvent = EventArray[index];
				ptr->wsabuf.buf = ptr->recvbuf + ptr->comp_recvbytes;
				ptr->wsabuf.len = ptr->recvbytes;

				DWORD recvbytes;

				flags = 0;
				retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
				if (retval == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						err_display("WSARecv()");
					}
				}

				continue;
			}
		}

		//가변길이 패킷부분을 다 받았을 경우
		if (ptr->packetRecvComple)
		{
			//초기화
			ptr->packetRecvComple = false;
			ptr->sizeRecvComple = false;

			//통신에 필요한 변수들
			RESULT join_result = NODATA;
			RESULT login_result = NODATA;
			char msg[BUFSIZE];
			PROTOCOL protocol;
			int size;

			//프로토콜 가져오기
			GetProtocol(ptr->recvbuf, protocol);

			//프로토콜에 따라 분리
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

			//상태에 따라 분리
			switch (ptr->state)
			{
				//회원가입
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

				//로그인
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


		//다시 가변길이패킷의 크기부분을 받게 하기 위해 WSARecv 하기전 전처리 부분
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->overlapped.hEvent = EventArray[index];
		ptr->wsabuf.buf = ptr->recvbuf;
		ptr->wsabuf.len = sizeof(int);

		DWORD recvbytes;

		//다시 가변길이패킷의 크기부분을 받는 부분
		retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				err_display("WSARecv()");
			}
		}
	}
}

_ClientInfo * AddClient(SOCKET _sock, SOCKADDR_IN _clientaddr)
{
	EnterCriticalSection(&cs);

	printf("\nClient 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(_clientaddr.sin_addr),
		ntohs(_clientaddr.sin_port));

	//소켓 구조체 배열에 새로운 소켓 정보 구조체 저장
	_ClientInfo * ptr = nullptr;

	WSAEVENT hEvent = WSACreateEvent();
	if (hEvent != WSA_INVALID_EVENT)
	{
		//인덱스 번호와 이벤트를 외부에서 구조체 안에 넣는다
		ptr = new _ClientInfo(hEvent, nTotalSockets);

		ptr->sock = _sock;
		memcpy(&ptr->addr, &_clientaddr, sizeof(SOCKADDR_IN));
		ptr->state = INIT_STATE;

		User_List[nTotalSockets] = ptr;
		EventArray[nTotalSockets] = hEvent;
		nTotalSockets++;
		LeaveCriticalSection(&cs);
		return ptr;
	}

	else
	{
		LeaveCriticalSection(&cs);
		return nullptr;
	}
}

void RemoveClient(int _index)
{
	EnterCriticalSection(&cs);
	
	_ClientInfo * ptr = User_List[_index];

	printf("\nClient 종료: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(ptr->addr.sin_addr), ntohs(ptr->addr.sin_port));

	closesocket(ptr->sock);
	WSACloseEvent(EventArray[_index]);
	delete User_List[_index];

	//배열 땡기기
	for (int i = _index; i < nTotalSockets - 1; i++)
	{
		User_List[i] = User_List[i + 1];
		User_List[i]->index = i;
	}

	User_List[nTotalSockets - 1] = nullptr;
	nTotalSockets--;

	LeaveCriticalSection(&cs);
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