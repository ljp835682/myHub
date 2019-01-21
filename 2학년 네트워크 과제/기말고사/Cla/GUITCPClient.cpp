#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512

enum PROTOCOL { INTRO, STRING, CREATEROOM, FINDROOM, CONNECTED };

BOOL CALLBACK LobbyProc(HWND, UINT, WPARAM, LPARAM);

BOOL CALLBACK CreateRoomProc(HWND, UINT, WPARAM, LPARAM);

BOOL CALLBACK FindRoomProc(HWND, UINT, WPARAM, LPARAM);
// 대화상자 프로시저
BOOL CALLBACK ChatProc(HWND, UINT, WPARAM, LPARAM);
// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...);
// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);
// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags);
// 소켓 통신 스레드 함수

void PackPacket(char*, PROTOCOL, char*, int&);
void PackPacket(char* _buf, PROTOCOL _protocol, char* _str1, char * _str2, int& _size);
void PackPacket(char* _buf, PROTOCOL _protocol, char* _str1, char * _str2, int num, int& _size);
void GetProtocol(char*, PROTOCOL&);
void UnPackPactket(char* _buf, char * _str);

DWORD WINAPI ClientMain(LPVOID arg);
DWORD WINAPI SendThread(LPVOID arg);
DWORD WINAPI RecvThread(LPVOID arg);
CRITICAL_SECTION cs;

SOCKET sock; // 소켓
char buf[BUFSIZE + 1]; // 데이터 송수신 버퍼
char idbuf[BUFSIZE + 1]; // 데이터 송수신 버퍼
char pwbuf[BUFSIZE + 1]; // 데이터 송수신 버퍼
HANDLE hReadEvent, hWriteEvent; // 이벤트
HWND hOKButton; // 보내기 버튼
HWND hEdit1, hEdit2, hEdit3; // 편집 컨트롤
HINSTANCE hInst;
HWND hMDlg;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
	InitializeCriticalSection(&cs);
	hInst = hInstance;
	// 이벤트 생성
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if(hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if(hWriteEvent == NULL) return 1;

	// 소켓 통신 스레드 생성
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	// 대화상자 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG4), NULL, LobbyProc);

	// 이벤트 제거
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);

	// closesocket()
	closesocket(sock);

	DeleteCriticalSection(&cs);

	// 윈속 종료
	WSACleanup();
	return 0;
}

BOOL CALLBACK LobbyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		PROTOCOL protocol;
		char ** idArray;
		//리시브
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		case IDC_CREATE_ROOM:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hDlg, CreateRoomProc);
			return TRUE;
		case IDC_FIND_ROOM:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hDlg, FindRoomProc);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

BOOL CALLBACK CreateRoomProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char packetbuf[BUFSIZE];

	switch (uMsg) {
	case WM_INITDIALOG:
		hEdit1 = GetDlgItem(hDlg, IDC_ROOMNAME1);
		hEdit2 = GetDlgItem(hDlg, IDC_ROOMPASS1);
		hEdit3 = GetDlgItem(hDlg, IDC_ROOMCOUNT);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
		SendMessage(hEdit2, EM_SETLIMITTEXT, BUFSIZE, 0);
		SendMessage(hEdit3, EM_SETLIMITTEXT, BUFSIZE, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemText(hDlg, IDC_ROOMNAME1, idbuf, BUFSIZE + 1);
			GetDlgItemText(hDlg, IDC_ROOMPASS1, pwbuf, BUFSIZE + 1);
			GetDlgItemText(hDlg, IDC_ROOMCOUNT, buf, BUFSIZE + 1);

			if (strlen(idbuf) != 0 && strlen(pwbuf) != 0)
			{
				int count = atoi(buf);

				if (count >= 2)
				{
					int size;
					PackPacket(packetbuf, CREATEROOM, idbuf, pwbuf, count, size);
					int retval = send(sock, packetbuf, size, 0);
					if (retval == SOCKET_ERROR) {
						err_display("send()");
						break;
					}

					PROTOCOL protocol;
					bool isCreated = false;
					//리시브

					if (isCreated)
					{
						DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hDlg, ChatProc);
					}
				}
			}

			ZeroMemory(idbuf, sizeof(idbuf));
			ZeroMemory(pwbuf, sizeof(pwbuf));
			ZeroMemory(buf, sizeof(buf));

			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

BOOL CALLBACK FindRoomProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char packetbuf[BUFSIZE];

	switch (uMsg) {
	case WM_INITDIALOG:
		hEdit1 = GetDlgItem(hDlg, IDC_ROOMNAME2);
		hEdit2 = GetDlgItem(hDlg, IDC_ROOMPASS2);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
		SendMessage(hEdit2, EM_SETLIMITTEXT, BUFSIZE, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemText(hDlg, IDC_ROOMNAME2, idbuf, BUFSIZE + 1);
			GetDlgItemText(hDlg, IDC_ROOMPASS2, pwbuf, BUFSIZE + 1);

			if (strlen(idbuf) != 0 && strlen(pwbuf) != 0)
			{
				int size;
				PackPacket(packetbuf, FINDROOM, idbuf, pwbuf, size);
				int retval = send(sock, packetbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("send()");
					break;
				}
				PROTOCOL protocol;
				bool isFind = false;
				//리시브

				if (isFind)
				{
					DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hDlg, ChatProc);
				}
			}

			ZeroMemory(idbuf, sizeof(idbuf));
			ZeroMemory(pwbuf, sizeof(pwbuf));

			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// 대화상자 프로시저
BOOL CALLBACK ChatProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_INITDIALOG:
		// 서버와 데이터 통신
		CreateThread(NULL, 0, SendThread, NULL, 0, NULL);
		CreateThread(NULL, 0, RecvThread, NULL, 0, NULL);

		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
		hOKButton = GetDlgItem(hDlg, IDOK);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDOK:
			EnableWindow(hOKButton, FALSE); // 보내기 버튼 비활성화
			WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 기다리기
			GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE+1);
			SetEvent(hWriteEvent); // 쓰기 완료 알리기
			SetFocus(hEdit1);
			SendMessage(hEdit1, EM_SETSEL, 0, -1);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char cbuf[BUFSIZE+256];
	vsprintf(cbuf, fmt, arg);

	int nLength = GetWindowTextLength(hEdit2);
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
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
	DisplayText("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while(left > 0){
		received = recv(s, ptr, left, flags);
		if(received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if(received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	while (1)
	{
		Sleep(0);
	}

	return 0;
}

// TCP 클라이언트 시작 부분
DWORD WINAPI SendThread(LPVOID arg)
{
	int retval;
	char packetbuf[BUFSIZE];
	ZeroMemory(packetbuf, sizeof(packetbuf));
	// 서버와 데이터 통신
	while (1) {
		WaitForSingleObject(hWriteEvent, INFINITE); // 쓰기 완료 기다리기
													// 문자열 길이가 0이면 보내지 않음
		if (strlen(buf) == 0) {
			SetEvent(hReadEvent); // 읽기 완료 알리기
			EnableWindow(hOKButton, TRUE); // 보내기 버튼 활성화
			continue;
		}

		// 데이터 보내기
		int size;
		PackPacket(packetbuf, STRING, buf, size);
		
		retval = send(sock, packetbuf, size, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		ZeroMemory(packetbuf, sizeof(packetbuf));

		EnableWindow(hOKButton, TRUE); // 보내기 버튼 활성화
		SetEvent(hReadEvent); // 읽기 완료 알리기
	}

	return 0;
}

DWORD WINAPI RecvThread(LPVOID arg)
{
	int retval = 0;
	char packetbuf[BUFSIZE];
	// 서버와 데이터 통신
	while (1) {
		// 데이터 받기
		int size;
		retval = recvn(sock, (char*)&size, sizeof(size), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("RecvError1()");
			break;

		}

		retval = recvn(sock, packetbuf, size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("RecvError1()");
			break;
		}

		PROTOCOL protocol;
		GetProtocol(packetbuf, protocol);

		char output[BUFSIZE];
		ZeroMemory(output, sizeof(output));
		UnPackPactket(packetbuf, output);

		// 받은 데이터 출력
		buf[retval] = '\0';
		DisplayText("[받은 데이터] %s\r\n", output);
	}

	return 0;
}

void PackPacket(char* _buf, PROTOCOL _protocol, char* _str1, char * _str2, int& _size)
{
	char* ptr = _buf;
	int strsize1 = strlen(_str1);
	int strsize2 = strlen(_str2);
	_size = sizeof(_protocol) + sizeof(strsize1) + strsize1 + sizeof(strsize2) + strsize2;

	memcpy(ptr, &_size, sizeof(_size));
	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);

	memcpy(ptr, &strsize1, sizeof(strsize1));
	ptr = ptr + sizeof(strsize1);

	memcpy(ptr, _str1, strsize1);
	ptr = ptr + strsize1;

	memcpy(ptr, &strsize2, sizeof(strsize2));
	ptr = ptr + sizeof(strsize2);

	memcpy(ptr, _str2, strsize2);
	ptr = ptr + strsize2;

	_size = _size + sizeof(_size);
}

void PackPacket(char* _buf, PROTOCOL _protocol, char* _str1, char * _str2, int num, int& _size)
{
	char* ptr = _buf;
	int strsize1 = strlen(_str1);
	int strsize2 = strlen(_str2);
	_size = sizeof(_protocol) + sizeof(strsize1) + strsize1 + sizeof(strsize2) + strsize2 + sizeof(num);

	memcpy(ptr, &_size, sizeof(_size));
	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);

	memcpy(ptr, &strsize1, sizeof(strsize1));
	ptr = ptr + sizeof(strsize1);

	memcpy(ptr, _str1, strsize1);
	ptr = ptr + strsize1;

	memcpy(ptr, &strsize2, sizeof(strsize2));
	ptr = ptr + sizeof(strsize2);

	memcpy(ptr, _str2, strsize2);
	ptr = ptr + strsize2;

	memcpy(ptr, &num, sizeof(num));
	ptr = ptr + sizeof(num);

	_size = _size + sizeof(_size);
}

void PackPacket(char* _buf, PROTOCOL _protocol, char* _str, int& _size)
{
	char* ptr = _buf;
	int strsize = strlen(_str);
	_size = sizeof(_protocol) + sizeof(strsize) + strsize;

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

void UnPackPactket(char* _buf, char * _str)
{
	char* ptr = _buf + sizeof(PROTOCOL);
	int strsize = 0;

	memcpy(&strsize, ptr, sizeof(strsize));
	ptr += sizeof(strsize);

	memcpy(_str, ptr, strsize);
	_str[strsize] = '\0';
	ptr += strsize;
}