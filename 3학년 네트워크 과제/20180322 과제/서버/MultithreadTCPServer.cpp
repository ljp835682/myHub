 #pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512
#define MAXUSER 20

#define PARK_ROOM "233.3.3.3"		
#define O_ROOM "233.3.3.4"		
#define KIM_ROOM "233.3.3.5"	

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
//enum STATE { INTRO_SEND, ROOM, CHAT, EXIT };

//Ŭ���̾�Ʈ ����ü
struct sClientInfo
{
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	//STATE state;
	HANDLE hthread;
	char  packetbuf[BUFSIZE];
	int id;
};

sClientInfo * ClientInfo[MAXUSER];
HANDLE hThread[MAXUSER];

int playerCount = 0;
int threadCount = 0;
unsigned int originalCode = 1;

CRITICAL_SECTION cs;

void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);

//Ŭ���̾�Ʈ ������
DWORD WINAPI ProcessClient(LPVOID arg);
//��� ������
DWORD CALLBACK WaitClient(LPVOID);

//Ŭ���̾�Ʈ �߰� �Լ�
sClientInfo * AddClient(SOCKET sock, SOCKADDR_IN clientaddr);
//Ŭ���̾�Ʈ ���� �Լ�
void RemoveClient(sClientInfo * ptr);

//������ �߰� �Լ�
void AddThread(LPTHREAD_START_ROUTINE process, LPVOID _ptr);
//������ ���� �Լ�
void RemoveThread(int index);

//��Ŷ ���� �Լ�
void PackPacket(char * buf, PROTOCOL protocol, int id, char * str1, char * str2, char * str3, int & size);
void PackPacket(char * buf, PROTOCOL protocol, SOCKADDR_IN addr, int & size);
void PackPacket(char * buf, PROTOCOL protocol, char * str1);

//�������� ���� �Լ�
void GetProtocol(char * ptr, PROTOCOL & protocol);

//��Ŷ ��ü �Լ�
void UnPackPactket(char * buf, int & data);
void UnPackPactket(char * buf, SOCKADDR_IN & addr);

int main(int argc, char *argv[])
{
	InitializeCriticalSection(&cs);

	int retval;

	// ���� �ʱ�ȭ
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

	// ������ ��ſ� ����� ����
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	int addrlen;

	hThread[threadCount] = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (hThread[threadCount] == NULL)
	{
		return 1;
	}

	else
	{
		threadCount++;
	}

	CreateThread(nullptr, 0, WaitClient, nullptr, 0, nullptr);

	while (1)
	{
		addrlen = sizeof(clientaddr);

		sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (sock == INVALID_SOCKET)
		{
			err_display("accept()");
			continue;
		}

		sClientInfo * ptr = AddClient(sock, clientaddr);
		AddThread(ProcessClient, ptr);
		SetEvent(hThread[0]);									//Ŭ���̾�Ʈ�� ���� �ٲ�� �˸������� ��ȣ��ȯ
	}

	// closesocket()
	closesocket(listen_sock);

	DeleteCriticalSection(&cs);

	// ���� ����
	WSACleanup();
	return 0;
}

DWORD CALLBACK WaitClient(LPVOID _ptr)
{
	while (1)
	{
		int index = WaitForMultipleObjects(threadCount, hThread, false, INFINITE);
		index -= WAIT_OBJECT_0;
		DWORD threadid;

		if (index == 0)				//���� 0�ϰ��
		{
			continue;				//hThread[0]�� ���� �̺�Ʈ�� �ڵ��̺�Ʈ�̹Ƿ� ������ ����
		}

		for (int i = 0; i < playerCount; i++)
		{
			HANDLE hthread = nullptr;

			if (ClientInfo[i]->hthread == hThread[index])
			{
				EnterCriticalSection(&cs);

				ClientInfo[i]->hthread = nullptr;

				RemoveClient(ClientInfo[i]);
				RemoveThread(index);

				LeaveCriticalSection(&cs);
				break;
			}
		}
	}
}

sClientInfo * AddClient(SOCKET sock, SOCKADDR_IN clientaddr)
{
	EnterCriticalSection(&cs);

	sClientInfo * ptr = new sClientInfo;
	ZeroMemory(ptr, sizeof(ptr));
	ptr->sock = sock;
	ptr->clientaddr = clientaddr;
	//ptr->state = INTRO_SEND;
	ptr->id = originalCode++;

	ClientInfo[playerCount++] = ptr;
	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));

	LeaveCriticalSection(&cs);
	return ptr;
}

void RemoveClient(sClientInfo * ptr)
{
	closesocket(ptr->sock);

	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(ptr->clientaddr.sin_addr),
		ntohs(ptr->clientaddr.sin_port));

	EnterCriticalSection(&cs);

	for (int i = 0; i < playerCount; i++)
	{
		if (ClientInfo[i] == ptr)
		{
			delete ptr;
			int j;
			for (j = i; j < playerCount - 1; j++)
			{
				ClientInfo[j] = ClientInfo[j + 1];
			}
			ClientInfo[j] = nullptr;
			break;
		}
	}

	playerCount--;
	LeaveCriticalSection(&cs);
}

void AddThread(LPTHREAD_START_ROUTINE process, LPVOID _ptr)
{
	EnterCriticalSection(&cs);
	sClientInfo* ptr = (sClientInfo*)_ptr;
	ptr->hthread = CreateThread(NULL, 0, process, ptr, 0, nullptr);
	hThread[threadCount++] = ptr->hthread;
	LeaveCriticalSection(&cs);
}

void RemoveThread(int index)
{
	EnterCriticalSection(&cs);
	int i;
	for (i = index; i < threadCount - 1; i++)
	{
		hThread[i] = hThread[i + 1];
	}

	hThread[i] = nullptr;
	threadCount--;
	LeaveCriticalSection(&cs);
}

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags)
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

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
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

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET udpSock;
	sClientInfo* ptr = (sClientInfo*)arg;
	int retval;
	int size = 0;
	int num;
	bool exitflag = false;

	// socket()
	udpSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSock == INVALID_SOCKET) err_quit("socket()");

	// ��Ƽĳ��Ʈ TTL ����
	int ttl = 2;
	retval = setsockopt(udpSock, IPPROTO_IP, IP_MULTICAST_TTL,
		(char *)&ttl, sizeof(ttl));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	//3���� �������� Ŭ���̾�Ʈ���� ����
	PackPacket(ptr->packetbuf, INTRO, ptr->id, "�ڿ���", "����ȯ", "������", size);
	retval = send(ptr->sock, ptr->packetbuf, size, 0);

	if (retval == SOCKET_ERROR)
	{
		err_display("send()");
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(ptr->clientaddr.sin_addr), ntohs(ptr->clientaddr.sin_port));
		return 0;
	}

	//�ݺ� recvn
	while (1) 
	{
		retval = recvn(ptr->sock, (char*)&size, sizeof(size), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("RecvError1()");
			break;

		}

		retval = recvn(ptr->sock, ptr->packetbuf, size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("RecvError1()");
			break;
		}

		PROTOCOL protocol;
		GetProtocol(ptr->packetbuf, protocol);

		switch (protocol)
		{
			//�� ����
		case ROOM_CHOICE:
			UnPackPactket(ptr->packetbuf, num);
			
			//���ο� �����ּ� ����ü ����
			SOCKADDR_IN addr;
			ZeroMemory(&addr, sizeof(addr));
			addr.sin_family = AF_INET;

			//�´°ɷ� �ʱ�ȭ
			switch (num)
			{
			case 0:
				addr.sin_port = htons(9001);
				addr.sin_addr.s_addr = inet_addr(PARK_ROOM);
				break;
			case 1:
				addr.sin_port = htons(9002);
				addr.sin_addr.s_addr = inet_addr(O_ROOM);
				break;
			case 2:
				addr.sin_port = htons(9003);
				addr.sin_addr.s_addr = inet_addr(KIM_ROOM);
				break;
			}
			
			//������ Ŭ�󿡰� send
			PackPacket(ptr->packetbuf, TICKET, addr, size);

			retval = send(ptr->sock, ptr->packetbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("send()");
				break;
			}

			ZeroMemory(ptr->packetbuf, BUFSIZE);

			//Ŭ�󿡰� ���� �ּҰ� ���ϴ°��� �ý��� �޼����� ������ send
			PackPacket(ptr->packetbuf, SYSTEM_MESSAGE, "������ ���Խ��ϴ�");

			retval = sendto(udpSock, ptr->packetbuf, BUFSIZE, 0, (SOCKADDR*)&addr, sizeof(addr));
			if (retval == SOCKET_ERROR)
			{
				err_display("sendto()2");
			}

			break;

			// ��Ż��
		case DODGE:
			//Ŭ�� �����ߴ� ��Ƽĳ��Ʈ �ּҸ� ���� ����ü�� �޾ƿ�
			SOCKADDR_IN remoteaddr;
			UnPackPactket(ptr->packetbuf, remoteaddr);
			ZeroMemory(ptr->packetbuf, BUFSIZE);

			//�װ��� �ý��� �޼����� ������ send
			PackPacket(ptr->packetbuf, SYSTEM_MESSAGE, "������ �������ϴ�");

			retval = sendto(udpSock, ptr->packetbuf, BUFSIZE, 0, (SOCKADDR*)&remoteaddr, sizeof(remoteaddr));
			if (retval == SOCKET_ERROR)
			{
				err_display("sendto()2");
			}

			break;

			//���α׷� ����
		case PROGRAM_QUIT:
			int exitUser;
			UnPackPactket(ptr->packetbuf, exitUser);

			//�������� ������ȣ�� �� �����带 ����ϴ� Ŭ���̾�Ʈ ����ü�� ������ȣ�� ������ ��
			if (exitUser == ptr->id)
			{
				exitflag = true;
			}

			break;
		}

		//�ݺ� recvn ����
		if (exitflag)
		{
			break;
		}
	}

	// closesocket()
	closesocket(udpSock);
	printf("\n[TCP ����] Ŭ���̾�Ʈ ������ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(ptr->clientaddr.sin_addr), ntohs(ptr->clientaddr.sin_port));

	return 0;
}

void PackPacket(char * buf, PROTOCOL protocol, int id, char * str1, char * str2, char * str3, int & size)
{
	char* ptr = buf;
	int strsize1 = strlen(str1);
	int strsize2 = strlen(str2);
	int strsize3 = strlen(str3);
	size = sizeof(protocol) + sizeof(id) + sizeof(strsize1) + strsize1 + sizeof(strsize2) + strsize2 + sizeof(strsize3) + strsize3;

	memcpy(ptr, &size, sizeof(size));
	ptr = ptr + sizeof(size);

	memcpy(ptr, &protocol, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	memcpy(ptr, &id, sizeof(id));
	ptr = ptr + sizeof(id);

	memcpy(ptr, &strsize1, sizeof(strsize1));
	ptr = ptr + sizeof(strsize1);

	memcpy(ptr, str1, strsize1);
	ptr = ptr + strsize1;

	memcpy(ptr, &strsize2, sizeof(strsize2));
	ptr = ptr + sizeof(strsize2);

	memcpy(ptr, str2, strsize2);
	ptr = ptr + strsize2;

	memcpy(ptr, &strsize3, sizeof(strsize3));
	ptr = ptr + sizeof(strsize3);

	memcpy(ptr, str3, strsize3);
	ptr = ptr + strsize3;

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

void PackPacket(char * buf, PROTOCOL protocol, char * str1)
{
	char* ptr = buf;
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

void UnPackPactket(char * buf, int & data)
{
	char* ptr = buf + sizeof(PROTOCOL);

	memcpy(&data, ptr, sizeof(data));
	ptr += sizeof(data);
}

void UnPackPactket(char * buf, SOCKADDR_IN & addr)
{
	char* ptr = buf + sizeof(PROTOCOL);
	int strsize1;

	memcpy(&addr, ptr, sizeof(addr));
	ptr += sizeof(addr);
}