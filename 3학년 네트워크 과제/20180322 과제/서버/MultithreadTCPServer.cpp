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
//enum STATE { INTRO_SEND, ROOM, CHAT, EXIT };

//클라이언트 구조체
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

//클라이언트 스레드
DWORD WINAPI ProcessClient(LPVOID arg);
//대기 스레드
DWORD CALLBACK WaitClient(LPVOID);

//클라이언트 추가 함수
sClientInfo * AddClient(SOCKET sock, SOCKADDR_IN clientaddr);
//클라이언트 삭제 함수
void RemoveClient(sClientInfo * ptr);

//스레드 추가 함수
void AddThread(LPTHREAD_START_ROUTINE process, LPVOID _ptr);
//스레드 삭제 함수
void RemoveThread(int index);

//패킷 조합 함수
void PackPacket(char * buf, PROTOCOL protocol, int id, char * str1, char * str2, char * str3, int & size);
void PackPacket(char * buf, PROTOCOL protocol, SOCKADDR_IN addr, int & size);
void PackPacket(char * buf, PROTOCOL protocol, char * str1);

//프로토콜 추출 함수
void GetProtocol(char * ptr, PROTOCOL & protocol);

//패킷 해체 함수
void UnPackPactket(char * buf, int & data);
void UnPackPactket(char * buf, SOCKADDR_IN & addr);

int main(int argc, char *argv[])
{
	InitializeCriticalSection(&cs);

	int retval;

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

	// 데이터 통신에 사용할 변수
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
		SetEvent(hThread[0]);									//클라이언트의 수가 바뀐걸 알리기위해 신호변환
	}

	// closesocket()
	closesocket(listen_sock);

	DeleteCriticalSection(&cs);

	// 윈속 종료
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

		if (index == 0)				//만약 0일경우
		{
			continue;				//hThread[0]에 만든 이벤트는 자동이벤트이므로 할일이 없다
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
	printf("\nClient 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));

	LeaveCriticalSection(&cs);
	return ptr;
}

void RemoveClient(sClientInfo * ptr)
{
	closesocket(ptr->sock);

	printf("\nClient 종료: IP 주소=%s, 포트 번호=%d\n",
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

// 사용자 정의 데이터 수신 함수
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

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

// 클라이언트와 데이터 통신
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

	// 멀티캐스트 TTL 설정
	int ttl = 2;
	retval = setsockopt(udpSock, IPPROTO_IP, IP_MULTICAST_TTL,
		(char *)&ttl, sizeof(ttl));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	//3개의 방제목을 클라이언트에게 보냄
	PackPacket(ptr->packetbuf, INTRO, ptr->id, "박용택", "오지환", "김현수", size);
	retval = send(ptr->sock, ptr->packetbuf, size, 0);

	if (retval == SOCKET_ERROR)
	{
		err_display("send()");
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(ptr->clientaddr.sin_addr), ntohs(ptr->clientaddr.sin_port));
		return 0;
	}

	//반복 recvn
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
			//방 선택
		case ROOM_CHOICE:
			UnPackPactket(ptr->packetbuf, num);
			
			//새로운 소켓주소 구조체 생성
			SOCKADDR_IN addr;
			ZeroMemory(&addr, sizeof(addr));
			addr.sin_family = AF_INET;

			//맞는걸로 초기화
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
			
			//조합후 클라에게 send
			PackPacket(ptr->packetbuf, TICKET, addr, size);

			retval = send(ptr->sock, ptr->packetbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("send()");
				break;
			}

			ZeroMemory(ptr->packetbuf, BUFSIZE);

			//클라에게 보낸 주소가 향하는곳에 시스템 메세지를 조합후 send
			PackPacket(ptr->packetbuf, SYSTEM_MESSAGE, "누군가 들어왔습니다");

			retval = sendto(udpSock, ptr->packetbuf, BUFSIZE, 0, (SOCKADDR*)&addr, sizeof(addr));
			if (retval == SOCKET_ERROR)
			{
				err_display("sendto()2");
			}

			break;

			// 방탈퇴
		case DODGE:
			//클라가 가입했던 멀티캐스트 주소를 담은 구조체를 받아옴
			SOCKADDR_IN remoteaddr;
			UnPackPactket(ptr->packetbuf, remoteaddr);
			ZeroMemory(ptr->packetbuf, BUFSIZE);

			//그곳에 시스템 메세지를 조합후 send
			PackPacket(ptr->packetbuf, SYSTEM_MESSAGE, "누군가 나갔습니다");

			retval = sendto(udpSock, ptr->packetbuf, BUFSIZE, 0, (SOCKADDR*)&remoteaddr, sizeof(remoteaddr));
			if (retval == SOCKET_ERROR)
			{
				err_display("sendto()2");
			}

			break;

			//프로그램 종료
		case PROGRAM_QUIT:
			int exitUser;
			UnPackPactket(ptr->packetbuf, exitUser);

			//나간유저 고유번호와 이 스레드를 사용하는 클라이언트 구조체의 고유번호가 같은지 비교
			if (exitUser == ptr->id)
			{
				exitflag = true;
			}

			break;
		}

		//반복 recvn 종료
		if (exitflag)
		{
			break;
		}
	}

	// closesocket()
	closesocket(udpSock);
	printf("\n[TCP 서버] 클라이언트 스레드 종료: IP 주소=%s, 포트 번호=%d\n",
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