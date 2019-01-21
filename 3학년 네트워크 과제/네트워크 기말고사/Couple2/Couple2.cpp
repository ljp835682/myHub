#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include "resource.h"
#define random(n) (rand()%n)

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512
#define NONCHOICE	-1

enum RESULT
{
	NODATA,
	TILE_FLIP_SUCCESE,
	TILE_FLIP_FAIL,
};

enum PROTOCOL
{
	INIT,
	GAME_END,
	TILE_SEND_REQ,
	TILE_SEND,
	MOUSE_POINT,
	TILE_FLIP_RESULT,
	TILE_TEMP_FLIP,
};


enum TILESTATE 
{ 
	HIDDEN,
	FLIP,
};

enum GAMESTATE 
{ 
	RUN, 
	HINT, 
	VIEW,
} gameState;

struct Tile
{
	int num;
	TILESTATE state;
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//메인 스레드
DWORD WINAPI ClientMain(LPVOID arg);
//리시브 전용 스레드
DWORD WINAPI RecvThread(LPVOID arg);

void err_quit(char *msg);
void err_display(char *msg);
bool PacketRecv(SOCKET _sock, char* _buf);
int recvn(SOCKET s, char *buf, int len, int flags);

SOCKET sock;
SOCKADDR_IN serveraddr;
char buf[BUFSIZE + 1];
HANDLE hReadEvent, hWriteEvent, hStartEvent;
bool isStart = false;
bool isEnd = false;
int x, y;

HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass=TEXT("Couple2");

Tile tiles[4][4];
int count;
HBITMAP hShape[9];

void DrawScreen(HDC hdc);
int GetRemain();
void DrawBitmap(HDC hdc,int x,int y,HBITMAP hBit);
void DeleteShape();

void GetProtocol(char * _buf, PROTOCOL & _protocol)
{
	memcpy(&_protocol, _buf, sizeof(PROTOCOL));
}

void UnPackPacket(char * _buf)
{
	char* ptr = _buf + sizeof(PROTOCOL);

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			memcpy(&tiles[i][j], ptr, sizeof(tiles[i][j]));
			ptr = ptr + sizeof(tiles[i][j]);
		}
	}
}

void UnPackPacket(char * _buf, int & _x, int & _y)
{
	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&_x, ptr, sizeof(_x));
	ptr = ptr + sizeof(_x);

	memcpy(&_y, ptr, sizeof(_y));
	ptr = ptr + sizeof(_y);
}

void UnPackPacket(char * _buf, RESULT & _result, int & _x1, int & _y1, int & _x2, int & _y2)
{
	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&_result, ptr, sizeof(_result));
	ptr = ptr + sizeof(_result);

	memcpy(&_x1, ptr, sizeof(_x1));
	ptr = ptr + sizeof(_x1);

	memcpy(&_y1, ptr, sizeof(_y1));
	ptr = ptr + sizeof(_y1);

	memcpy(&_x2, ptr, sizeof(_x2));
	ptr = ptr + sizeof(_x2);

	memcpy(&_y2, ptr, sizeof(_y2));
	ptr = ptr + sizeof(_y2);
}

void PackPacket(char * _buf, PROTOCOL _protocol, int & _size)
{
	char* ptr = _buf;
	_size = 0;

	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);

	ptr = _buf;
	memcpy(ptr, &_size, sizeof(_size));

	_size = _size + sizeof(_size);
}

void PackPacket(char * _buf, PROTOCOL _protocol, int _x, int _y, int & _size)
{
	char* ptr = _buf;
	_size = 0;

	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);

	memcpy(ptr, &_x, sizeof(_x));
	ptr = ptr + sizeof(_x);
	_size = _size + sizeof(_x);

	memcpy(ptr, &_y, sizeof(_y));
	ptr = ptr + sizeof(_y);
	_size = _size + sizeof(_y);

	ptr = _buf;
	memcpy(ptr, &_size, sizeof(_size));

	_size = _size + sizeof(_size);
}


LRESULT CALLBACK WndProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT crt;

	switch (iMessage) {
	case WM_CREATE:
		SetRect(&crt,1,1,(64*4+250) + 1,(64*4) + 1);
		AdjustWindowRect(&crt,WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,FALSE);
		SetWindowPos(hWnd,NULL,0,0,crt.right-crt.left,crt.bottom-crt.top,
			SWP_NOMOVE | SWP_NOZORDER);
		hWndMain=hWnd;

		for (int i=0;i<sizeof(hShape)/sizeof(hShape[0]);i++)
		{
			hShape[i]=LoadBitmap(g_hInst,MAKEINTRESOURCE(IDB_SHAPE1+i));
		}
		return 0;

	case WM_LBUTTONDOWN:
		if (!isStart)
		{
			MessageBox(hWnd, "게임이 아직 시작되지 않았습니다", "알림", MB_OK);
			return 0;
		}

		x = LOWORD(lParam);
		y = HIWORD(lParam);

		if (x >= 1 && x <= (64 * 4 + 250) + 1 && y >= 1 && y <= (64 * 4) + 1)
		{
			SetEvent(hWriteEvent); // 읽기 완료 알리기
		}

		return 0;

	case WM_TIMER:
		return 0;

	case WM_PAINT:
		hdc=BeginPaint(hWnd, &ps);
		DrawScreen(hdc);
		EndPaint(hWnd, &ps);
		return 0;

	case WM_DESTROY:
		DeleteShape();
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd,iMessage,wParam,lParam));
}

void DrawScreen(HDC hdc)
{
	int x,y,image;
	TCHAR Mes[128];

	for (x=0;x<4;x++) {
		for (y=0;y<4;y++) {
			if (gameState ==HINT || tiles[x][y].state!=HIDDEN) {
				image=tiles[x][y].num-1;
			} else {
				image=8;
			}
			DrawBitmap(hdc,(x*64) + 1,(y*64) + 1,hShape[image]);
		}
	}

	lstrcpy(Mes,"짝 맞추기 게임 Ver 1.2");
	TextOut(hdc,300,10,Mes,lstrlen(Mes));
	wsprintf(Mes,"총 시도 회수 : %d",count);
	TextOut(hdc,300,50,Mes,lstrlen(Mes));
	wsprintf(Mes,"아직 못 찾은 것 : %d   ",GetRemain());
	TextOut(hdc,300,70,Mes,lstrlen(Mes));
}

int GetRemain()
{
	int i,j;
	int remain=16;

	for (i=0;i<4;i++) {
		for (j=0;j<4;j++) {
			if (tiles[i][j].state == FLIP) {
				remain--;
			}
		}
	}
	return remain;
}

void DrawBitmap(HDC hdc,int x,int y,HBITMAP hBit)
{
	HDC MemDC;
	HBITMAP OldBitmap;
	int bx,by;
	BITMAP bit;

	MemDC=CreateCompatibleDC(hdc);
	OldBitmap=(HBITMAP)SelectObject(MemDC, hBit);

	GetObject(hBit,sizeof(BITMAP),&bit);
	bx=bit.bmWidth;
	by=bit.bmHeight;

	BitBlt(hdc,x,y,bx,by,MemDC,0,0,SRCCOPY);

	SelectObject(MemDC,OldBitmap);
	DeleteDC(MemDC);
}

void DeleteShape()
{
	for (int i = 0; i<sizeof(hShape) / sizeof(hShape[0]); i++) 
	{
		DeleteObject(hShape[i]);
	}
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
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	char packetbuf[BUFSIZE];
	ZeroMemory(packetbuf, sizeof(packetbuf));

	PROTOCOL protocol = TILE_SEND_REQ;
	int size;
	PackPacket(packetbuf, protocol, size);

	retval = send(sock, packetbuf, size, 0);
	if (retval == SOCKET_ERROR) {
		err_quit("send()");
	}

	CreateThread(NULL, 0, RecvThread, NULL, 0, NULL);
	WaitForSingleObject(hStartEvent, INFINITE); // 시작 기다리기
	isStart = true;

	//이 아래는 send용 무한반복문
	while (1)
	{
		WaitForSingleObject(hWriteEvent, INFINITE); // 쓰기 완료 기다리기

		protocol = MOUSE_POINT;
		// 데이터 보내기
		PackPacket(packetbuf, protocol, x, y, size);

		retval = send(sock, packetbuf, size, 0);
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}

		SetEvent(hReadEvent); // 읽기 완료 알리기
	}

	return 0;
}


DWORD WINAPI RecvThread(LPVOID arg)
{
	PROTOCOL protocol;
	RESULT result;
	int x1, y1, x2, y2;
	char packetbuf[BUFSIZE];
	ZeroMemory(packetbuf, sizeof(packetbuf));

	while (1)
	{
		//데이터 수신
		if (!PacketRecv(sock, packetbuf))
		{
			break;
		}

		//프로토콜 분리
		GetProtocol(packetbuf, protocol);

		switch (protocol)
		{
		case TILE_SEND:
			UnPackPacket(packetbuf);
			SetEvent(hStartEvent);
			InvalidateRect(hWndMain, NULL, TRUE);
			break;

		case GAME_END:
			isEnd = true;
			MessageBox(NULL, "파트너가 게임을 종료하였습니다", "알림", MB_OK);
			DeleteShape();
			PostQuitMessage(0);
			break;

		case TILE_TEMP_FLIP:
			UnPackPacket(packetbuf, x1, y1);
			tiles[x1][y1].state = FLIP;
			InvalidateRect(hWndMain, NULL, TRUE);
			break;

		case TILE_FLIP_RESULT:
			UnPackPacket(packetbuf, result, x1, y1, x2, y2);

			switch (result)
			{
			case TILE_FLIP_SUCCESE:
				tiles[x1][y1].state = FLIP;
				tiles[x2][y2].state = FLIP;
				InvalidateRect(hWndMain, NULL, TRUE);
				break;

			case TILE_FLIP_FAIL:
				tiles[x1][y1].state = FLIP;
				tiles[x2][y2].state = FLIP;
				InvalidateRect(hWndMain, NULL, TRUE);
				Sleep(1000);
				tiles[x1][y1].state = HIDDEN;
				tiles[x2][y2].state = HIDDEN;
				InvalidateRect(hWndMain, NULL, TRUE);
				break;
			}
			break;
		}
	}

	return 0;
}

void err_quit(char * msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(char * msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

bool PacketRecv(SOCKET _sock, char * _buf)
{
	int size;

	int retval = recvn(_sock, (char*)&size, sizeof(size), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("recv error()");
		return false;
	}
	else if (retval == 0)
	{
		return false;
	}

	retval = recvn(_sock, _buf, size, 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("recv error()");
		return false;

	}
	else if (retval == 0)
	{
		return false;
	}

	return true;
}

int recvn(SOCKET s, char * buf, int len, int flags)
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

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL) return 1;
	hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hStartEvent == NULL) return 1;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_COUPLE));
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = 0;
	RegisterClass(&WndClass);

	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return (int)Message.wParam;
}