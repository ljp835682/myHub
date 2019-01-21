#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include "resource.h"

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512
#define NONCHOICE	-1

/*
프로토콜
INTRO				인트로
ROOM_CHOICE			방선택
TICKET				방주소
MESSAGE				문자열송수신
EXIT				클라이언트가 채팅방을 나가고 자신에게 보낼 방탈출
DODGE				클라이언트가 채팅방을 나가고 서버에게 보낼 방탈출
SYSTEM_MESSAGE		알림메세지
PROGRAM_QUIT		클라이언트가 프로그램을 종료
*/

enum PROTOCOL { INTRO, ROOM_CHOICE, TICKET, MESSAGE, EXIT, DODGE, SYSTEM_MESSAGE, PROGRAM_QUIT };

//방 선택 다이얼로그
BOOL CALLBACK DlgProc1(HWND, UINT, WPARAM, LPARAM);
//채팅방 다이얼로그
BOOL CALLBACK DlgProc2(HWND, UINT, WPARAM, LPARAM);

//메인 스레드
DWORD WINAPI ClientMain(LPVOID arg);
//리시브 전용 스레드
DWORD WINAPI RecvThread(LPVOID arg);

void DisplayText(char *fmt, ...);
void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);

//패킷 조합 함수
void PackPacket(char * buf, PROTOCOL protocol, SOCKADDR_IN addr, int & size);
void PackPacket(char * buf, PROTOCOL protocol, int data, int & size);
void PackPacket(char * buf, PROTOCOL protocol, int id);
void PackPacket(char * buf, PROTOCOL protocol, char * str1);

//프로토콜 추출 함수
void GetProtocol(char * buf, PROTOCOL & protocol);

//패킷 해체 함수
void UnPackPactket(char * buf, int & id, char * str1, char * str2, char * str3);
void UnPackPactket(char * buf, SOCKADDR_IN & addr);
void UnPackPactket(char * buf, char * str1);
void UnPackPactket(char * buf, int & data);

SOCKET sock;				//TCP용 소켓
SOCKET udpSock;				//UDP용 소켓
char buf[BUFSIZE+1];
HANDLE hReadEvent, hWriteEvent;
HWND hOKButton;
HWND hEdit1, hEdit2, hEdit3;
HINSTANCE hins;
int myID = 0;
SOCKADDR_IN multicastaddr, localaddr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
	hins = hInstance;

	// 이벤트 생성
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if(hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(hWriteEvent == NULL) return 1;

	// 소켓 통신 스레드 생성
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	// 대화상자 생성
	DialogBox(hins, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc1);

	// 이벤트 제거
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);
	
	// 윈속 종료
	WSACleanup();
	return 0;
}

//방 선택 다이얼로그
BOOL CALLBACK DlgProc1(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int retval;
	static int size;
	static char packetbuf[BUFSIZE + 1] = "";
	static int num = 0;

	switch (uMsg) {

	//폼 생성시
	case WM_INITDIALOG:
		//리스트박스를 1번 윈도우핸들에 지정
		hEdit1 = GetDlgItem(hDlg, IDC_LIST2);
		
		//인트로를 받아옴
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

		if (protocol == INTRO)
		{
			//인트로 해체후 방이름 입력
			char buf1[30] = "", buf2[30] = "", buf3[30] = "";
			UnPackPactket(packetbuf, myID, buf1, buf2, buf3);
			SendMessage(hEdit1, LB_ADDSTRING, 0, (LPARAM)buf1);
			SendMessage(hEdit1, LB_ADDSTRING, 0, (LPARAM)buf2);
			SendMessage(hEdit1, LB_ADDSTRING, 0, (LPARAM)buf3);
		}

		return TRUE;

	//메세지 발생시
	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{

		//확인 버튼
		case IDOK:
			
			//지정된 방이름 번호
			num = SendMessage(hEdit1, LB_GETCURSEL, 0, 0);

			//선택된게 있을경우
			if (num != NONCHOICE)
			{
				//선택한 방번호 조합후 송신
				PackPacket(packetbuf, ROOM_CHOICE, num, size);

				retval = send(sock, packetbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("send()");
					break;
				}

				//멀티캐스트 주소 구소체 받아옴
				retval = recvn(sock, (char*)&size, sizeof(size), 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("RecvError1()");
					break;

				}

				retval = recvn(sock, packetbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("RecvError2()");
					break;
				}

				PROTOCOL protocol;
				GetProtocol(packetbuf, protocol);

				if (protocol == TICKET)
				{
					UnPackPactket(packetbuf, multicastaddr);
					DialogBox(hins, MAKEINTRESOURCE(IDD_DIALOG2), hDlg, DlgProc2);
				}
			}

			return TRUE;

		//취소 버튼
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;

	case WM_DESTROY:
		//서버의 스레드를 종료시킬 패킷을 전송
		PackPacket(packetbuf, PROGRAM_QUIT, myID, size);

		retval = send(sock, packetbuf, size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}

		// closesocket()
		closesocket(sock);
		break;
		
	}
	return FALSE;
}

// 대화상자 프로시저
BOOL CALLBACK DlgProc2(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int retval;
	static int addrlen;
	static char packetbuf[BUFSIZE + 1] = "";
	static bool flag = false;
	static struct ip_mreq mreq;
	static BOOL optval = true;
	switch (uMsg) 
	{

	//폼 생성시
	case WM_INITDIALOG:
		
		//소켓 생성
		udpSock = socket(AF_INET, SOCK_DGRAM, 0);
		if (udpSock == INVALID_SOCKET) err_quit("udpSocket()");

		retval = setsockopt(udpSock, SOL_SOCKET, SO_REUSEADDR,
			(char *)&optval, sizeof(optval));
		if (retval == SOCKET_ERROR) err_quit("setudpSockopt1()");

		//bind()
		ZeroMemory(&localaddr, sizeof(localaddr));
		localaddr.sin_family = AF_INET;
		localaddr.sin_port = multicastaddr.sin_port;
		localaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		retval = bind(udpSock, (SOCKADDR *)&localaddr, sizeof(localaddr));
		if (retval == SOCKET_ERROR) err_quit("udpbind()"); 

		//멀티캐스트 그룹 가입
		memcpy(&mreq.imr_multiaddr.s_addr, 
			&multicastaddr.sin_addr, sizeof(multicastaddr.sin_addr));
		mreq.imr_interface.s_addr = inet_addr("127.0.0.1");

		//멀티캐스트 TTL 설정
		retval = setsockopt(udpSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			(char *)&mreq, sizeof(mreq));
		if (retval == SOCKET_ERROR) err_quit("setudpSockopt2()");

		//리시브 스레드 생성
		CreateThread(NULL, 0, RecvThread, NULL, 0, NULL);
		
		//텍스트창을 핸들에 지정
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit3 = GetDlgItem(hDlg, IDC_EDIT2);

		//확인버튼을 핸들에 지정
		hOKButton = GetDlgItem(hDlg, IDOK);
		SendMessage(hEdit2, EM_SETLIMITTEXT, BUFSIZE, 0);
		return TRUE;

	//메세지 발생시
	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		//확인 버튼
		case IDOK:
			EnableWindow(hOKButton, FALSE); // 보내기 버튼 비활성화
			WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 기다리기
			GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
			SetEvent(hWriteEvent); // 쓰기 완료 알리기
			SetFocus(hEdit2);
			SendMessage(hEdit2, EM_SETSEL, 0, -1);
			return TRUE;

		//취소 버튼
		case IDCANCEL:
			//리시브 스레드 종료용 패킷 조합후 send
			PackPacket(packetbuf, EXIT, myID);

			retval = sendto(udpSock, packetbuf, BUFSIZE, 0, (SOCKADDR*)&multicastaddr, sizeof(multicastaddr));
			if (retval == SOCKET_ERROR) 
			{
				err_display("sendto()");
			}

			ZeroMemory(packetbuf, sizeof(packetbuf));

			int size = 0;
			
			//서버에 이 클라이언트가 채팅방을 나갔다는걸 알리기위한 패킷을 조합후 send
			PackPacket(packetbuf, DODGE, multicastaddr, size);

			retval = send(sock, packetbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("send()");
				break;
			}

			ZeroMemory(packetbuf, sizeof(packetbuf));
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
		
	//폼 종료시
	case WM_DESTROY:
		//멀티캐스트 그룹 탈퇴
		retval = setsockopt(udpSock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
			(char *)&mreq, sizeof(mreq));
		if (retval == SOCKET_ERROR) err_quit("setsockopt()");

		//udp 소켓 종료
		closesocket(udpSock);

		return TRUE;
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

	int nLength = GetWindowTextLength(hEdit3);
	SendMessage(hEdit3, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit3, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

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

	char packetbuf[BUFSIZE];
	ZeroMemory(packetbuf, sizeof(packetbuf));
	
	//이 아래는 send용 무한반복문
	while (1)
	{
		WaitForSingleObject(hWriteEvent, INFINITE); // 쓰기 완료 기다리기
													// 문자열 길이가 0이면 보내지 않음
		if (strlen(buf) == 0) {
			SetEvent(hReadEvent); // 읽기 완료 알리기
			EnableWindow(hOKButton, TRUE); // 보내기 버튼 활성화
			continue;
		}

		// 데이터 보내기
		PackPacket(packetbuf, MESSAGE, buf);

		retval = sendto(udpSock, packetbuf, BUFSIZE, 0, (SOCKADDR*)&multicastaddr, sizeof(multicastaddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}

		EnableWindow(hOKButton, TRUE); // 보내기 버튼 활성화
		SetEvent(hReadEvent); // 읽기 완료 알리기
	}

	return 0;
}

DWORD WINAPI RecvThread(LPVOID arg)
{
	SOCKADDR_IN peeraddr;
	int addrlen = sizeof(peeraddr);
	int retval = 0;
	char packetbuf[BUFSIZE + 1];
	bool exitFlag = false;
	char output[BUFSIZE];
	int size;
	ZeroMemory(packetbuf, BUFSIZE + 1);

	//무한 recvfrom
	while (1) 
	{
		//데이터 수신
		retval = recvfrom(udpSock, packetbuf,sizeof(packetbuf),0, (SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			break;
		}

		//프로토콜 분리
		PROTOCOL protocol;
		GetProtocol(packetbuf, protocol);
		

		switch (protocol)
		{

		//시스템 메세지 일경우
		case SYSTEM_MESSAGE:
			ZeroMemory(output, sizeof(output));
			UnPackPactket(packetbuf, output);

			// 받은 데이터 출력
			DisplayText("[알림] %s\r\n", output);
			break;

		//일반 메세지 일경우
		case MESSAGE:
			ZeroMemory(output, sizeof(output));
			UnPackPactket(packetbuf, output);

			// 받은 데이터 출력
			DisplayText("[참가자] %s\r\n", output);
			break;

		//리시브 스레드를 종료하라는 프로토콜 일경우
		case EXIT:
			int id;
			UnPackPactket(packetbuf, id);

			//자신이 보낸 패킷이지 확인
			if (myID == id)
			{
				//자신이 보낸거면 종료플래그를 세움
				exitFlag = true;
			}
			break;
		}

		//종료플래그가 세워져있다면 무한 recvfrom을 종료하고 스레드함수가 종료
		if (exitFlag)
		{
			break;
		}
	}

	return 0;
}

void PackPacket(char * buf, PROTOCOL protocol, int data, int & size)
{
	char* ptr = buf;
	size = sizeof(protocol) + sizeof(data);

	memcpy(ptr, &size, sizeof(size));
	ptr = ptr + sizeof(size);

	memcpy(ptr, &protocol, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	memcpy(ptr, &data, sizeof(data));
	ptr = ptr + sizeof(data);

	size = size + sizeof(size);
}

void PackPacket(char * buf, PROTOCOL protocol, SOCKADDR_IN addr, int & size)
{
	char* ptr = buf;
	size = sizeof(protocol) + sizeof(addr);

	memcpy(ptr, &size, sizeof(size));
	ptr = ptr + sizeof(size);

	memcpy(ptr, &protocol, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	memcpy(ptr, &addr, sizeof(addr));
	ptr = ptr + sizeof(addr);

	size = size + sizeof(size);
}

void PackPacket(char * buf, PROTOCOL protocol, int id)
{
	char* ptr = buf;

	memcpy(ptr, &protocol, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	memcpy(ptr, &id, sizeof(id));
	ptr = ptr + sizeof(id);

}

void PackPacket(char * buf, PROTOCOL protocol, char * str1)
{
	char * ptr = buf;
	int strsize1 = strlen(str1);

	memcpy(ptr, &protocol, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	memcpy(ptr, &strsize1, sizeof(strsize1));
	ptr = ptr + sizeof(strsize1);

	memcpy(ptr, str1, strsize1);
	ptr = ptr + strsize1;
}


void GetProtocol(char * ptr, PROTOCOL & protocol)
{
	memcpy(&protocol, ptr, sizeof(PROTOCOL));
}

void UnPackPactket(char * buf, int & id, char * str1, char * str2, char * str3)
{
	char* ptr = buf + sizeof(PROTOCOL);
	int strsize1, strsize2, strsize3;

	memcpy(&id, ptr, sizeof(id));
	ptr += sizeof(id);

	memcpy(&strsize1, ptr, sizeof(strsize1));
	ptr += sizeof(strsize1);

	memcpy(str1, ptr, strsize1);
	ptr += strsize1;

	memcpy(&strsize2, ptr, sizeof(strsize2));
	ptr += sizeof(strsize2);

	memcpy(str2, ptr, strsize2);
	ptr += strsize2;

	memcpy(&strsize3, ptr, sizeof(strsize3));
	ptr += sizeof(strsize3);

	memcpy(str3, ptr, strsize3);
	ptr += strsize3;

	str1[strsize1] = '\0';
	str2[strsize2] = '\0';
	str3[strsize3] = '\0';
}

void UnPackPactket(char * buf, SOCKADDR_IN & addr)
{
	char* ptr = buf + sizeof(PROTOCOL);
	int strsize1;

	memcpy(&addr, ptr, sizeof(addr));
	ptr += sizeof(addr);
}

void UnPackPactket(char * buf, char * str1)
{
	char * ptr = buf + sizeof(PROTOCOL);
	int strsize1 = 0;

	memcpy(&strsize1, ptr, sizeof(strsize1));
	ptr += sizeof(strsize1);

	memcpy(str1, ptr, strsize1);
	ptr += strsize1;

	str1[strsize1] = '\0';
}

void UnPackPactket(char * buf, int & data)
{
	char* ptr = buf + sizeof(PROTOCOL);

	memcpy(&data, ptr, sizeof(data));
	ptr += sizeof(data);
}