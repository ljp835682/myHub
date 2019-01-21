#include "List.h"

int who_win(int part1, int part2);
void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);

_ClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr);
void RemoveClient(_ClientInfo* ptr);

DWORD CALLBACK ProcessClient(LPVOID);
DWORD CALLBACK WaitClient(LPVOID);

void AddThread(LPTHREAD_START_ROUTINE process, LPVOID _ptr);
void RemoveThread(int index);

void PackPacket(char*, PROTOCOL, char*, int&);
void GetProtocol(char*, PROTOCOL&);
void UnPackPactket(char* _buf, char * _str);


CRITICAL_SECTION cs;
int main(int argc, char **argv)
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;
	InitializeCriticalSection(&cs);
	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	int retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수		
	int addrlen;
	SOCKET sock;
	SOCKADDR_IN clientaddr;

	hThread[threadcount] = CreateEvent(NULL, FALSE, FALSE, NULL);					//자동 이벤트 생성
													
	if (hThread[threadcount] == NULL)
	{
		return 1;
	}

	else
	{
		threadcount++;
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

		_ClientInfo* ptr = AddClient(sock, clientaddr);
		AddThread(ProcessClient, ptr);
		SetEvent(hThread[0]);									//클라이언트의 수가 바뀐걸 알리기위해 신호변환
	}

	CloseHandle(hThread[0]);									//0번  이벤트의 이벤트를 삭제하는 부분 이부분은 절대 올수없다
	closesocket(listen_sock);
	DeleteCriticalSection(&cs);
	WSACleanup();
	return 0;
}

void AddThread(LPTHREAD_START_ROUTINE process, LPVOID _ptr)
{
	EnterCriticalSection(&cs);
	_ClientInfo* ptr = (_ClientInfo*)_ptr;
	ptr->hthread = CreateThread(NULL, 0, process, ptr, 0, nullptr);
	hThread[threadcount++] = ptr->hthread;
	LeaveCriticalSection(&cs);
}

void RemoveThread(int index)
{
	EnterCriticalSection(&cs);
	int i;
	for (i = index; i < threadcount - 1; i++)
	{
		hThread[i] = hThread[i + 1];
	}

	hThread[i] = nullptr;
	threadcount--;
	LeaveCriticalSection(&cs);
}

DWORD CALLBACK ProcessClient(LPVOID  _ptr)
{
	_ClientInfo* Client_ptr = (_ClientInfo*)_ptr;

	char* result_str = nullptr;
	int size;
	int strsize;
	PROTOCOL protocol;
	int result;
	int retval;
	
	char buf[BUFSIZE];

	while (1) {
		// 데이터 받기
		retval = recvn(Client_ptr->sock, (char*)&size, sizeof(size), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("RecvError1()");
			break;

		}

		retval = recvn(Client_ptr->sock, Client_ptr->packetbuf, size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("RecvError1()");
			break;

		}

		PROTOCOL protocol;
		GetProtocol(Client_ptr->packetbuf, protocol);

		char output[BUFSIZE] = "test";
		ZeroMemory(output, sizeof(output));
		UnPackPactket(Client_ptr->packetbuf, output);

		// 받은 데이터 출력
		printf("[TCP/%s:%d] %s\r\n", inet_ntoa(Client_ptr->clientaddr.sin_addr),
			ntohs(Client_ptr->clientaddr.sin_port), output);

		PackPacket(Client_ptr->packetbuf, STRING, output, size);

		ZeroMemory(Client_ptr->packetbuf, sizeof(Client_ptr->packetbuf));
		
		//클래스안의 데이터 보내기 함수 호출

		// 데이터 보내기
	}

	return 0;
}

void PackPacket(char* _buf, PROTOCOL _protocol, char* _str, int& _size)
{
	char* ptr = _buf;
	int strsize = strlen(_str);
	_size = sizeof(_protocol)+sizeof(strsize)+strsize;

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

DWORD CALLBACK WaitClient(LPVOID _ptr)
{
	while (1)
	{
		int index = WaitForMultipleObjects(threadcount, hThread, false, INFINITE);
		index -= WAIT_OBJECT_0;
		DWORD threadid;

		if (index == 0)				//만약 0일경우
		{
			continue;				//hThread[0]에 만든 이벤트는 자동이벤트이므로 할일이 없다
		}

		for (int i = 0; i < count; i++)
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

void err_quit(char *msg)
{
	LPVOID lpMsgbuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgbuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgbuf, msg, MB_ICONERROR);
	LocalFree(lpMsgbuf);
	exit(-1);
}

int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0){
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

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgbuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgbuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgbuf);
	LocalFree(lpMsgbuf);
}

int who_win(int part1, int part2)
{
	if (part1 == part2)
		return NO_WIN;
	else if (part1 % 3 == (part2 + 1) % 3)
		return FIRST_WIN;
	else
		return SECOND_WIN;
}

_ClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr)
{
	EnterCriticalSection(&cs);
	_ClientInfo* ptr = new _ClientInfo;
	ZeroMemory(ptr, sizeof(_ClientInfo));
	ptr->sock = sock;
	memcpy(&(ptr->clientaddr), &clientaddr, sizeof(clientaddr));
	ptr->eventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);					//수동 이벤트 생성
	ptr->catchHandle = CreateEvent(NULL, TRUE, FALSE, NULL);					//수동 이벤트 생성
	ptr->endhandle = CreateEvent(NULL, TRUE, FALSE, NULL);						//수동 이벤트 생성

	ClientInfo[count] = ptr;
	printf("\nClient 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));
	count++;
	LeaveCriticalSection(&cs);
	return ptr;
}

void RemoveClient(_ClientInfo* ptr)
{
	closesocket(ptr->sock);

	printf("\nClient 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(ptr->clientaddr.sin_addr),
		ntohs(ptr->clientaddr.sin_port));

	EnterCriticalSection(&cs);				

	CloseHandle(ptr->eventHandle);							//이벤트  이벤트 삭제

	for (int i = 0; i < count; i++)
	{
		if (ClientInfo[i] == ptr)
		{		
			delete ptr;
			int j;
			for (j = i; j < count - 1; j++)
			{
				ClientInfo[j] = ClientInfo[j + 1];
			}
			ClientInfo[j] = nullptr;
			break;
		}
	}

	count--;
	LeaveCriticalSection(&cs);
}