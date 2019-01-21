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

	//외부에서 자신의 배열속 인덱스 번호와 자신과 매치되는 이벤트를 가져옴
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
	printf("\nClient 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(_clientaddr.sin_addr),
		ntohs(_clientaddr.sin_port));

	//인덱스 번호와 이벤트를 외부에서 구조체 안에 넣는다
	_ClientInfo * ptr = new _ClientInfo();

	ptr->sock = _sock;
	memcpy(&ptr->addr, &_clientaddr, sizeof(SOCKADDR_IN));
	ptr->state = INIT_STATE;
	return ptr;
}

void RemoveClient(_ClientInfo * _ptr)
{
	printf("\nClient 종료: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(_ptr->addr.sin_addr), ntohs(_ptr->addr.sin_port));
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

// 비동기 입출력 시작과 처리 함수
DWORD WINAPI WorkerThread(LPVOID arg);

// WorkerThread가 끝나면 호출된 루틴
void CALLBACK CompletionRoutine(
	DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags
);
// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);
void err_display(int errcode);

int main(int argc, char *argv[])
{
	int retval;

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

	// 이벤트 객체 생성
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if(hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(hWriteEvent == NULL) return 1;

	// 스레드 생성
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

	// 윈속 종료
	WSACleanup();
	return 0;
}

// 비동기 입출력 시작 함수
//누군가가 들어오면 최초의 리시브만을 날려주는 역활
DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;

	while(1){
		while(1){
			// alertable wait
			//WaitForSingleObject"Ex" : 처음에는 cpu시간을 안받지만 os 가 완료 루틴을 호출하면서 여기서 완료루틴을 실행한다
			//그 완료 루틴이 끝나야 이 함수가 리턴한다

			//리턴되는 경우
			//1. hWriteEvent 이벤트가 켜지는 경우
			//2. 완료루틴이 끝나서 완료루틴이 끝났다는 의미로 켜지는경우
			DWORD result = WaitForSingleObjectEx(hWriteEvent, INFINITE, TRUE);
			
			//hWriteEvent 가 켜진경우
			if (result == WAIT_OBJECT_0) break;

			//완료루틴이 끝난경우 메인을 거쳐서 다시 WaitForSingleObjectEx을 실행
			if (result != WAIT_IO_COMPLETION) return 1;
		}

		// 접속한 클라이언트 정보 출력
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		
		getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 소켓 정보 구조체 할당과 초기화
	 	_ClientInfo * ptr = AddClient(client_sock, clientaddr);

		SetEvent(hReadEvent);
		ptr->recvbytes = sizeof(int);
		ptr->WSARecvBuf();

		// 비동기 입출력 시작
		DWORD recvbytes;
		DWORD flags = 0;

		//최초의 리시브
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

// 비동기 입출력 처리 함수(입출력 완료 루틴)
void CALLBACK CompletionRoutine(
	DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	DWORD recvbytes;
	DWORD flags = 0;
	int retval;

	// 클라이언트 정보 얻기
	_ClientInfo * ptr = (_ClientInfo *)lpOverlapped;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR *)&clientaddr, &addrlen);

	// 비동기 입출력 결과 확인
	if(dwError != 0 || cbTransferred == 0){
		if(dwError != 0) err_display(dwError);
		closesocket(ptr->sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		RemoveClient(ptr);
		return;
	}

	//패킷의 사이즈를 받는부분
	if (!ptr->sizeRecvComple)
	{
		//받은 사이즈를 더한다
		ptr->comp_recvbytes += cbTransferred;

		//전부다 받았을경우
		if (ptr->comp_recvbytes == ptr->recvbytes)
		{
			//초기화
			ptr->comp_recvbytes = 0;
			ptr->sizeRecvComple = true;

			//앞으로 받아야할 사이즈를 버퍼에서 추출
			memcpy(&ptr->recvbytes, ptr->recvbuf, sizeof(int));	//크기를 복사
			ptr->WSARecvBuf();

			//새로운 리시브
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

		//전부다 받지 못할경우
		else
		{
			//recvbytes의 크기를 유지한채 속행한다
			ptr->WSARecvBuf();

			//이어서 리시브
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

		//버그가 없도록 이 루틴을 끝냄
		return;
	}

	//패킷의 내용을 받는부분
	if(!ptr->packetRecvComple && ptr->sizeRecvComple)
	{
		//받은 사이즈를 더한다
		ptr->comp_recvbytes += cbTransferred;

		//전부 전송 되엇을경우
		if (ptr->comp_recvbytes == ptr->recvbytes)
		{
			//초기화
			ptr->recvbytes = sizeof(int);
			ptr->comp_recvbytes = 0;

			//다받은걸 알림
			ptr->packetRecvComple = true;
		}

		//다 받지 못한경우
		else
		{
			//recvbytes의 크기를 유지한채 속행한다
			ptr->WSARecvBuf();

			//이어서 리시브
			retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, CompletionRoutine);
			if (retval == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					err_display("WSARecv()");
					return;
				}
			}

			//위의 가변길이패킷의 사이즈를 받는 if문 과는 다르게 else문에만 루틴탈출
			return;
		}
	}

	//모든 패킷을 받는게 끝났을경우
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

// 소켓 함수 오류 출력
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[오류] %s", (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}