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
��������
INTRO				��Ʈ��
ROOM_CHOICE			�漱��
TICKET				���ּ�
MESSAGE				���ڿ��ۼ���
EXIT				Ŭ���̾�Ʈ�� ä�ù��� ������ �ڽſ��� ���� ��Ż��
DODGE				Ŭ���̾�Ʈ�� ä�ù��� ������ �������� ���� ��Ż��
SYSTEM_MESSAGE		�˸��޼���
PROGRAM_QUIT		Ŭ���̾�Ʈ�� ���α׷��� ����
*/

enum PROTOCOL { INTRO, ROOM_CHOICE, TICKET, MESSAGE, EXIT, DODGE, SYSTEM_MESSAGE, PROGRAM_QUIT };

//�� ���� ���̾�α�
BOOL CALLBACK DlgProc1(HWND, UINT, WPARAM, LPARAM);
//ä�ù� ���̾�α�
BOOL CALLBACK DlgProc2(HWND, UINT, WPARAM, LPARAM);

//���� ������
DWORD WINAPI ClientMain(LPVOID arg);
//���ú� ���� ������
DWORD WINAPI RecvThread(LPVOID arg);

void DisplayText(char *fmt, ...);
void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);

//��Ŷ ���� �Լ�
void PackPacket(char * buf, PROTOCOL protocol, SOCKADDR_IN addr, int & size);
void PackPacket(char * buf, PROTOCOL protocol, int data, int & size);
void PackPacket(char * buf, PROTOCOL protocol, int id);
void PackPacket(char * buf, PROTOCOL protocol, char * str1);

//�������� ���� �Լ�
void GetProtocol(char * buf, PROTOCOL & protocol);

//��Ŷ ��ü �Լ�
void UnPackPactket(char * buf, int & id, char * str1, char * str2, char * str3);
void UnPackPactket(char * buf, SOCKADDR_IN & addr);
void UnPackPactket(char * buf, char * str1);
void UnPackPactket(char * buf, int & data);

SOCKET sock;				//TCP�� ����
SOCKET udpSock;				//UDP�� ����
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

	// �̺�Ʈ ����
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if(hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(hWriteEvent == NULL) return 1;

	// ���� ��� ������ ����
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	// ��ȭ���� ����
	DialogBox(hins, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc1);

	// �̺�Ʈ ����
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);
	
	// ���� ����
	WSACleanup();
	return 0;
}

//�� ���� ���̾�α�
BOOL CALLBACK DlgProc1(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int retval;
	static int size;
	static char packetbuf[BUFSIZE + 1] = "";
	static int num = 0;

	switch (uMsg) {

	//�� ������
	case WM_INITDIALOG:
		//����Ʈ�ڽ��� 1�� �������ڵ鿡 ����
		hEdit1 = GetDlgItem(hDlg, IDC_LIST2);
		
		//��Ʈ�θ� �޾ƿ�
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
			//��Ʈ�� ��ü�� ���̸� �Է�
			char buf1[30] = "", buf2[30] = "", buf3[30] = "";
			UnPackPactket(packetbuf, myID, buf1, buf2, buf3);
			SendMessage(hEdit1, LB_ADDSTRING, 0, (LPARAM)buf1);
			SendMessage(hEdit1, LB_ADDSTRING, 0, (LPARAM)buf2);
			SendMessage(hEdit1, LB_ADDSTRING, 0, (LPARAM)buf3);
		}

		return TRUE;

	//�޼��� �߻���
	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{

		//Ȯ�� ��ư
		case IDOK:
			
			//������ ���̸� ��ȣ
			num = SendMessage(hEdit1, LB_GETCURSEL, 0, 0);

			//���õȰ� �������
			if (num != NONCHOICE)
			{
				//������ ���ȣ ������ �۽�
				PackPacket(packetbuf, ROOM_CHOICE, num, size);

				retval = send(sock, packetbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("send()");
					break;
				}

				//��Ƽĳ��Ʈ �ּ� ����ü �޾ƿ�
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

		//��� ��ư
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;

	case WM_DESTROY:
		//������ �����带 �����ų ��Ŷ�� ����
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

// ��ȭ���� ���ν���
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

	//�� ������
	case WM_INITDIALOG:
		
		//���� ����
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

		//��Ƽĳ��Ʈ �׷� ����
		memcpy(&mreq.imr_multiaddr.s_addr, 
			&multicastaddr.sin_addr, sizeof(multicastaddr.sin_addr));
		mreq.imr_interface.s_addr = inet_addr("127.0.0.1");

		//��Ƽĳ��Ʈ TTL ����
		retval = setsockopt(udpSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			(char *)&mreq, sizeof(mreq));
		if (retval == SOCKET_ERROR) err_quit("setudpSockopt2()");

		//���ú� ������ ����
		CreateThread(NULL, 0, RecvThread, NULL, 0, NULL);
		
		//�ؽ�Ʈâ�� �ڵ鿡 ����
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit3 = GetDlgItem(hDlg, IDC_EDIT2);

		//Ȯ�ι�ư�� �ڵ鿡 ����
		hOKButton = GetDlgItem(hDlg, IDOK);
		SendMessage(hEdit2, EM_SETLIMITTEXT, BUFSIZE, 0);
		return TRUE;

	//�޼��� �߻���
	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		//Ȯ�� ��ư
		case IDOK:
			EnableWindow(hOKButton, FALSE); // ������ ��ư ��Ȱ��ȭ
			WaitForSingleObject(hReadEvent, INFINITE); // �б� �Ϸ� ��ٸ���
			GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
			SetEvent(hWriteEvent); // ���� �Ϸ� �˸���
			SetFocus(hEdit2);
			SendMessage(hEdit2, EM_SETSEL, 0, -1);
			return TRUE;

		//��� ��ư
		case IDCANCEL:
			//���ú� ������ ����� ��Ŷ ������ send
			PackPacket(packetbuf, EXIT, myID);

			retval = sendto(udpSock, packetbuf, BUFSIZE, 0, (SOCKADDR*)&multicastaddr, sizeof(multicastaddr));
			if (retval == SOCKET_ERROR) 
			{
				err_display("sendto()");
			}

			ZeroMemory(packetbuf, sizeof(packetbuf));

			int size = 0;
			
			//������ �� Ŭ���̾�Ʈ�� ä�ù��� �����ٴ°� �˸������� ��Ŷ�� ������ send
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
		
	//�� �����
	case WM_DESTROY:
		//��Ƽĳ��Ʈ �׷� Ż��
		retval = setsockopt(udpSock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
			(char *)&mreq, sizeof(mreq));
		if (retval == SOCKET_ERROR) err_quit("setsockopt()");

		//udp ���� ����
		closesocket(udpSock);

		return TRUE;
	}
	return FALSE;
}

// ���� ��Ʈ�� ��� �Լ�
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
	DisplayText("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// ����� ���� ������ ���� �Լ�
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

	// ���� �ʱ�ȭ
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
	
	//�� �Ʒ��� send�� ���ѹݺ���
	while (1)
	{
		WaitForSingleObject(hWriteEvent, INFINITE); // ���� �Ϸ� ��ٸ���
													// ���ڿ� ���̰� 0�̸� ������ ����
		if (strlen(buf) == 0) {
			SetEvent(hReadEvent); // �б� �Ϸ� �˸���
			EnableWindow(hOKButton, TRUE); // ������ ��ư Ȱ��ȭ
			continue;
		}

		// ������ ������
		PackPacket(packetbuf, MESSAGE, buf);

		retval = sendto(udpSock, packetbuf, BUFSIZE, 0, (SOCKADDR*)&multicastaddr, sizeof(multicastaddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}

		EnableWindow(hOKButton, TRUE); // ������ ��ư Ȱ��ȭ
		SetEvent(hReadEvent); // �б� �Ϸ� �˸���
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

	//���� recvfrom
	while (1) 
	{
		//������ ����
		retval = recvfrom(udpSock, packetbuf,sizeof(packetbuf),0, (SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			break;
		}

		//�������� �и�
		PROTOCOL protocol;
		GetProtocol(packetbuf, protocol);
		

		switch (protocol)
		{

		//�ý��� �޼��� �ϰ��
		case SYSTEM_MESSAGE:
			ZeroMemory(output, sizeof(output));
			UnPackPactket(packetbuf, output);

			// ���� ������ ���
			DisplayText("[�˸�] %s\r\n", output);
			break;

		//�Ϲ� �޼��� �ϰ��
		case MESSAGE:
			ZeroMemory(output, sizeof(output));
			UnPackPactket(packetbuf, output);

			// ���� ������ ���
			DisplayText("[������] %s\r\n", output);
			break;

		//���ú� �����带 �����϶�� �������� �ϰ��
		case EXIT:
			int id;
			UnPackPactket(packetbuf, id);

			//�ڽ��� ���� ��Ŷ���� Ȯ��
			if (myID == id)
			{
				//�ڽ��� �����Ÿ� �����÷��׸� ����
				exitFlag = true;
			}
			break;
		}

		//�����÷��װ� �������ִٸ� ���� recvfrom�� �����ϰ� �������Լ��� ����
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