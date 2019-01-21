#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "LinkedList.h"

#define SERVERPORT 9000
#define WM_SOCKET  (WM_USER+1)

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

// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);
void err_display(int errcode);

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

	int								recvbytes;
	int								comp_recvbytes;
	int								sendbytes;
	int								comp_sendbytes;

	char							recvbuf[BUFSIZE];
	char							sendbuf[BUFSIZE];

	_ClientInfo()
	{
		temp_user = new _User_Info();

		pre_state = INIT_STATE;
		state = INIT_STATE;

		recvbytes = 0;
		comp_recvbytes = 0;
		sendbytes = 0;
		comp_sendbytes = 0;

		ZeroMemory(recvbuf, 0, sizeof(recvbuf));
		ZeroMemory(sendbuf, 0, sizeof(sendbuf));
	}

	~_ClientInfo()
	{
		delete temp_user;
	}
};

CLinkedList<_ClientInfo *> * User_List;

CLinkedList<_User_Info *> * Join_List;

// 윈도우 메시지 처리 함수
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ProcessSocketMessage(HWND, UINT, WPARAM, LPARAM);
// 소켓 관리 함수

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

BOOL SearchFile(char *filename)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile = FindFirstFile(filename, &FindFileData);
	if (hFindFile == INVALID_HANDLE_VALUE)
		return FALSE;
	else {
		FindClose(hFindFile);
		return TRUE;
	}
}

bool FileDataLoad()
{
	if (!SearchFile("UserInfo.info"))
	{
		FILE* fp = fopen("UserInfo.info", "wb");
		fclose(fp);
		return true;
	}

	FILE* fp = fopen("UserInfo.info", "rb");
	if (fp == NULL)
	{
		return false;
	}

	_User_Info info;
	memset(&info, 0, sizeof(_User_Info));

	while (1)
	{
		fread(&info, sizeof(_User_Info), 1, fp);
		if (feof(fp))
		{
			break;
		}
		_User_Info* ptr = new _User_Info;
		memcpy(ptr, &info, sizeof(_User_Info));
		Join_List->Insert(ptr);
	}

	fclose(fp);
	return true;
}

bool FileDataAdd(_User_Info* _info)
{
	FILE* fp = fopen("UserInfo.info", "ab");
	if (fp == NULL)
	{
		return false;
	}

	int retval = fwrite(_info, 1, sizeof(_User_Info), fp);

	if (retval != sizeof(_User_Info))
	{
		fclose(fp);
		return false;
	}

	fclose(fp);
	return true;
}

int MessageRecv(_ClientInfo* _info, int _size)
{
	_info->recvbytes = _size;
	int retval = recv(_info->sock,
		_info->recvbuf + _info->comp_recvbytes,
		_info->recvbytes - _info->comp_recvbytes, 0);
	if (retval == SOCKET_ERROR) //강제연결종료요청인 경우
	{
		return ERROR_DISCONNECTED;
	}
	else if (retval == 0)
	{
		return DISCONNECTED;
	}
	else
	{
		_info->comp_recvbytes = _info->comp_recvbytes + retval;
		if (_info->comp_recvbytes == _info->recvbytes)
		{
			_info->comp_recvbytes = 0;
			_info->recvbytes = 0;
			return SOC_TRUE;
		}
		return SOC_FALSE;
	}

}

int MessageSend(_ClientInfo* _info, int _size)
{

	_info->sendbytes = _size;
	int retval = send(_info->sock, _info->sendbuf + _info->comp_sendbytes,
		_info->sendbytes - _info->comp_sendbytes, 0);
	if (retval == SOCKET_ERROR) //강제연결종료요청인 경우
	{
		return ERROR_DISCONNECTED;
	}
	else if (retval == 0)
	{
		DISCONNECTED;
	}
	else
	{
		_info->comp_sendbytes = _info->comp_sendbytes + retval;

		if (_info->sendbytes == _info->comp_sendbytes)
		{
			_info->sendbytes = 0;
			_info->comp_sendbytes = 0;

			return SOC_TRUE;
		}
		else
		{
			return SOC_FALSE;
		}
	}
}

int PacketRecv(_ClientInfo* _ptr)
{

	if (_ptr->recvbytes < sizeof(int))
	{
		int retval = MessageRecv(_ptr, sizeof(int));
		switch (retval)
		{
		case SOC_TRUE:
			memcpy(&_ptr->recvbytes,
				_ptr->recvbuf, sizeof(int));
			return SOC_FALSE;
		case SOC_FALSE:
			return SOC_FALSE;
		case ERROR_DISCONNECTED:
			err_display("recv error()");
			return DISCONNECTED;
		case DISCONNECTED:
			return DISCONNECTED;
		}
	}

	int retval = MessageRecv(_ptr, _ptr->recvbytes);
	switch (retval)
	{
	case SOC_TRUE:
		return SOC_TRUE;
	case SOC_FALSE:
		return SOC_FALSE;
	case ERROR_DISCONNECTED:
		err_display("recv error()");
		return DISCONNECTED;
	case DISCONNECTED:
		return DISCONNECTED;
	}
}

_ClientInfo* AddClient(SOCKET _sock, SOCKADDR_IN _clientaddr)
{
	printf("\nClient 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(_clientaddr.sin_addr),
		ntohs(_clientaddr.sin_port));

	//소켓 구조체 배열에 새로운 소켓 정보 구조체 저장
	_ClientInfo* ptr = new _ClientInfo();

	ptr->sock = _sock;
	memcpy(&ptr->addr, &_clientaddr, sizeof(SOCKADDR_IN));
	ptr->state = INIT_STATE;

	User_List->Insert(ptr);
	return ptr;
}

void RemoveClient(_ClientInfo* _ptr)
{
	closesocket(_ptr->sock);

	printf("\nClient 종료: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(_ptr->addr.sin_addr), ntohs(_ptr->addr.sin_port));

	User_List->Delete(_ptr);
}

_ClientInfo * FindClient(SOCKET _sock)
{
	_ClientInfo * iter = nullptr;

	User_List->SearchStart();

	while (1)
	{
		iter = User_List->SearchData();

		if (iter == nullptr)
		{
			break;
		}

		else if (iter->sock == _sock)
		{
			break;
		}
	}

	User_List->SearchEnd();
	return iter;
}

int main(int argc, char *argv[])
{
	int protocolsize = sizeof(PROTOCOL);
	User_List = new CLinkedList<_ClientInfo *>();
	Join_List = new CLinkedList<_User_Info *>();

	int retval;

	// 윈도우 클래스 등록
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "MyWndClass";
	if(!RegisterClass(&wndclass)) return 1;

	// 윈도우 생성
	HWND hWnd = CreateWindow("MyWndClass", "TCP 서버", WS_OVERLAPPEDWINDOW,
		0, 0, 600, 200, NULL, NULL, NULL, NULL);
	if(hWnd == NULL) return 1;
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

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

	// WSAAsyncSelect()
	retval = WSAAsyncSelect(listen_sock, hWnd,
		WM_SOCKET, FD_ACCEPT|FD_CLOSE);
	if(retval == SOCKET_ERROR) err_quit("WSAAsyncSelect()");

	// 메시지 루프
	MSG msg;
	while(GetMessage(&msg, 0, 0, 0) > 0){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 윈속 종료
	WSACleanup();

	delete User_List;
	delete Join_List;

	return msg.wParam;
}

// 윈도우 메시지 처리
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_SOCKET: // 소켓 관련 윈도우 메시지
		ProcessSocketMessage(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//WSAAsyncSelect함수를 사용하기 쉽게 매핑해둔 함수
void WSAAsyncSelectMapping(_ClientInfo * _ptr, HWND _hwnd, bool _isWrite)
{
	int retval = 0;

	//_isWrite 의 값에 따라 FD_WRITE 를 넣을지 말지를 정함
	if (_isWrite)
	{
		retval = WSAAsyncSelect(_ptr->sock, _hwnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
	}

	else
	{
		retval = WSAAsyncSelect(_ptr->sock, _hwnd, WM_SOCKET, FD_READ | FD_CLOSE);
	}
	
	//에러
	if (retval == SOCKET_ERROR)
	{
		err_display("WSAAsyncSelect()");
		RemoveClient(_ptr);
	}
}

// 소켓 관련 윈도우 메시지 처리
void ProcessSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//해당 소켓을가진 클라이언트 구조체를 찾아냄 못찾아내면 nullptr을 반환함
	_ClientInfo * ptr = FindClient(wParam);

	// 데이터 통신에 사용할 변수
	int addrlen, retval;

	// 오류 발생 여부 확인
	if(WSAGETSELECTERROR(lParam)){
		err_display(WSAGETSELECTERROR(lParam));
		RemoveClient(ptr);
		return;
	}

	//클라이언트 구조체를 찾아냇을경우에만 WSAAsyncSelect Write 없이
	if (ptr != nullptr)
	{
		WSAAsyncSelectMapping(ptr, hWnd, false);
	}

	int size = 0;
	char msg[BUFSIZE];
	PROTOCOL protocol;

	// 메시지 처리
	switch (WSAGETSELECTEVENT(lParam)) {

		//Accept()
	case FD_ACCEPT:
	{
		SOCKET client_sock;
		SOCKADDR_IN clientaddr;
		addrlen = sizeof(clientaddr);
		client_sock = accept(wParam, (SOCKADDR *)&clientaddr, &addrlen);

		if (client_sock == INVALID_SOCKET)
		{
			err_display("accept()");
			return;
		}

		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		ptr = AddClient(client_sock, clientaddr);
		ptr->state = LOGIN_MENU_SELECT_STATE;

		retval = WSAAsyncSelect(client_sock, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);

		if (retval == SOCKET_ERROR)
		{
			err_display("WSAAsyncSelect()");
			RemoveClient(ptr);
		}
		break;
	}
		//recv()
	case FD_READ:
	{

		//recv를 실행
		int result = PacketRecv(ptr);

		switch (result)
		{
		case DISCONNECTED:
			ptr->state = DISCONNECTED_STATE;
			break;
		case SOC_FALSE:
			return;
		case SOC_TRUE:
			break;
		}

		//현재상태에따라 구분
		switch (ptr->state)
		{
		case LOGIN_MENU_SELECT_STATE:

			GetProtocol(ptr->recvbuf, protocol);


			//send를 해야할경우 WSAAsyncSelectMapping(ptr, hWnd, true); 를 호출합니다
			switch (protocol)
			{
			case JOIN_INFO:
				UnPackPacket(ptr->recvbuf, ptr->temp_user->id, ptr->temp_user->pw, ptr->temp_user->nickname);
				ptr->state = JOIN_STATE;
				WSAAsyncSelectMapping(ptr, hWnd, true);

				break;
			case LOGIN_INFO:
				UnPackPacket(ptr->recvbuf, ptr->temp_user->id, ptr->temp_user->pw);
				ptr->state = LOGIN_STATE;
				WSAAsyncSelectMapping(ptr, hWnd, true);
				break;
			}
			break;
		}

		break;
	}
		//send()
	case FD_WRITE:
	{
		RESULT join_result = NODATA;
		RESULT login_result = NODATA;

		switch (ptr->state)
		{
		case JOIN_STATE:

			Join_List->SearchStart();

			while (1)
			{
				_User_Info* user_info = Join_List->SearchData();
				if (user_info == nullptr)
				{
					break;
				}
				if (!strcmp(user_info->id, ptr->temp_user->id))
				{
					join_result = ID_EXIST;
					strcpy(msg, ID_EXIST_MSG);
					break;
				}
			}

			Join_List->SearchEnd();

			if (join_result == NODATA)
			{
				_User_Info* user = new _User_Info;
				memset(user, 0, sizeof(_User_Info));
				strcpy(user->id, ptr->temp_user->id);
				strcpy(user->pw, ptr->temp_user->pw);
				strcpy(user->nickname, ptr->temp_user->nickname);

				FileDataAdd(user);

				Join_List->Insert(user);
				join_result = JOIN_SUCCESS;
				strcpy(msg, JOIN_SUCCESS_MSG);
			}

			protocol = JOIN_RESULT;

			PackPacket(ptr->sendbuf, protocol, join_result, msg, size);

			retval = send(ptr->sock, ptr->sendbuf, size, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				ptr->state = DISCONNECTED_STATE;
				break;
			}

			ptr->state = LOGIN_MENU_SELECT_STATE;
			break;
		case LOGIN_STATE:
			Join_List->SearchStart();
			while (1)
			{
				_User_Info* user_info = Join_List->SearchData();
				if (user_info == nullptr)
				{
					break;
				}
				if (!strcmp(user_info->id, ptr->temp_user->id))
				{
					if (!strcmp(user_info->pw, ptr->temp_user->pw))
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

			Join_List->SearchEnd();

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

			if (login_result == LOGIN_SUCCESS)
			{
				ptr->state = LOGIN_MENU_SELECT_STATE;
			}
			break;
		}

		//다시 함수가 호출되면 WSAAsyncSelectMapping(ptr, hWnd, false); 가 호출되므로
		//여기서 WSAAsyncSelectMapping(ptr, hWnd, true); 를 호출할 필요는 없을것
		break;
	}
	case FD_CLOSE:
		RemoveClient(ptr);
		break;
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