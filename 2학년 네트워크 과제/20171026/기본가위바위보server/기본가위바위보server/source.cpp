#pragma comment(lib, "ws2_32")
#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define BUFSIZE 512
#define PLAYERCOUNT 10
#define NODATA -1

enum { NO_WIN, FIRST_WIN, SECOND_WIN = -1 };
enum STATE{ INTRO_SEND, GVALUE_RECV, RESULT_SEND, GAME_OVER, DISCONNECT };
enum PROTOCOL{ INTRO, GVALUE, RESULT };

struct _ClientInfo
{
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	STATE state;
	int	gvalue;
	HANDLE thread;
	char  packetbuf[BUFSIZE];
	_ClientInfo* part;
};

HANDLE hThread[PLAYERCOUNT + 1];
_ClientInfo* ClientInfo[PLAYERCOUNT];
int count = 0;
int th_count = 0;

int who_win(int part1, int part2);
void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);
_ClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr);
void RemoveClient(_ClientInfo* ptr);
bool MatchPartner(_ClientInfo* _ptr);
void AddThread(_ClientInfo* _ptr);
void RemoveThread(int index);
bool SendFunction(char * _str, _ClientInfo* _player, PROTOCOL _protocol);
bool RecvFunction(_ClientInfo * _player);
DWORD CALLBACK ProcessClient(LPVOID);
DWORD WINAPI Dummy(LPVOID arg);
DWORD WINAPI WaitingThread(LPVOID arg);


char No_win[] = "비겼습니다.!!";
char win[] = "이겼습니다!!";
char lose[] = "졌습니다!!!!";
char Intro[] = "가위:0, 바위:1, 보:2 를 입력하세요:";

int main(int argc, char **argv)
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;
	
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

	hThread[th_count++] = CreateThread(NULL, 0, Dummy, 0, 0, NULL);								//더미 스레드 함수 생성
	HANDLE wating = CreateThread(NULL, 0, WaitingThread, 0, 0, NULL);					//감시용 스레드 함수 생성
	CloseHandle(wating);																//감시용 스레드 함수 권리 포기

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
		MatchPartner(ptr);
		AddThread(ptr);
		
		if (ptr->thread == NULL)
		{
			RemoveClient(ptr);
			continue;
		}

		else
		{			
			TerminateThread(hThread[0], 0);												//더미 스레드 함수 터미네이트
			th_count++;
		}
	}

	closesocket(listen_sock);
	
	WSACleanup();
	return 0;
}

DWORD WINAPI WaitingThread(LPVOID arg)
{
	while (1)
	{
		int index = WaitForMultipleObjects(th_count, hThread, false, INFINITE);		//th_count만큼 량의 스레드 함수를 감시한다

		if (index == 0)																//만약 더미 스레드 함수가 죽엇다면
		{
			hThread[0] = CreateThread(NULL, 0, Dummy, 0, 0, NULL);					//새로운 함수가 들어온것임
		}																			//다시 WaitForMultipleObjects 를 활성하 하기위해 더미 스레드 함수를 만들어준다

		else if (index == -1)
		{
			printf("index 값이 -1 오류발생\n");
			continue;
		}

		else
		{
			printf("%d번 스레드 종료\n", index);									//그외에는 게임 스레드 함수가 종료된것

			_ClientInfo * ptr = nullptr;											

			for (int i = 0; i < count; i++)
			{
				if (ClientInfo[i]->thread == hThread[index])
				{
					ptr = ClientInfo[i];
					break;
				}
			}

			if (ptr == nullptr)
			{
				printf("종료된 스레드는 존재하지만 삭제할 클라이언트가 존재하지않음(오류)\n");
				continue;
			}

			switch (ptr->state)
			{
			case GAME_OVER:
				RemoveClient(ptr);
				break;
			case RESULT_SEND:
				
				break;
			case DISCONNECT:
				if (ptr->part != nullptr)
				{
					if (MatchPartner(ptr->part))
					{
						if (ptr->part == nullptr)
						{
							AddThread(ptr->part);
						}

						if (ptr->part->part == nullptr)
						{
							AddThread(ptr->part->part);
						}
					}

					else
					{
						ptr->part->part = nullptr;
					}
				}
				break;
			}

			RemoveThread(index);
		}
	}
}

DWORD WINAPI Dummy(LPVOID arg)
{
	while (1)
	{
		Sleep(0);
	}

	return ERROR;
}

DWORD CALLBACK ProcessClient(LPVOID arg)
{
	_ClientInfo* Client_ptr = (_ClientInfo*)arg;

	char* result_str = nullptr;
	int result;

	bool endflag = false;
	while (1)
	{
		switch (Client_ptr->state)
		{
		case INTRO_SEND:
			if (!SendFunction(Intro, Client_ptr, INTRO))
			{
				Client_ptr->state = DISCONNECT;
				return -1;
			}

			else
			{
				Client_ptr->state = GVALUE_RECV;
			}

			break;
		case GVALUE_RECV:
			if (!RecvFunction(Client_ptr))
			{
				Client_ptr->state = DISCONNECT;
				return -1;
			}

			Client_ptr->state = RESULT_SEND;
			break;
		case RESULT_SEND:

			if (Client_ptr->part != nullptr && Client_ptr->part->gvalue!=NODATA)
			{
				result = who_win(Client_ptr->gvalue, Client_ptr->part->gvalue);

				switch (result)
				{
				case FIRST_WIN:
					result_str = win;
					break;
				case SECOND_WIN:
					result_str = lose;
					break;
				case NO_WIN:
					result_str = No_win;
					break;
				}

				if (!SendFunction(result_str, Client_ptr, RESULT))
				{
					Client_ptr->state = DISCONNECT;
					return -1;
				}

				Client_ptr->state = GAME_OVER;
				break;
			}
			break;

		case GAME_OVER:
			endflag = true;
			break;
		}

		if (endflag)
		{
			break;
		}
	}
	return 0;
}

bool SendFunction(char * _str, _ClientInfo* _player, PROTOCOL _protocol)
//인자 : 클라이언트에게 보낼 문자열,클라이언트 정보를 담은 _ClientInfo 구조체 포인터,어떤 형식의 패킷인지 구분할 프로토콜
{
	int str_size = strlen(_str);											//전송할 문자열의 길이를 저장합니다
	char * ptr = _player->packetbuf;										//전송할 클라이언트의 버퍼에 데이터를 쓸 준비를 합니다 
	int size = sizeof(_protocol);					//패킷 버퍼의 전체크기를 계산합니다
	//프로토콜의 크기 + 문자열의 길이를 담은 int형 변수의 크기 + 문자열 길이

	switch (_protocol)														//프로토콜로 스위치문을 실행하여
	{
	case INTRO:
	case RESULT:
		size += +sizeof(str_size)+str_size;
		break;
	}

	memcpy(ptr, &size, sizeof(size));										//포인터의 위치에 사이즈를 복사
	ptr = ptr + sizeof(size);												//포인터 사이즈의 크기만큼 이동

	memcpy(ptr, &_protocol, sizeof(_protocol));								//포인터의 위치에 프로토콜을 복사
	ptr = ptr + sizeof(_protocol);											//포인터 프로토콜의 크기만큼 이동


	switch (_protocol)
	{
	case INTRO:
	case RESULT:
		memcpy(ptr, &str_size, sizeof(str_size));								//포인터의 위치에 문자열의 크기를 복사
		ptr = ptr + sizeof(str_size);											//포인터 문자열의 크기의 크기만큼 이동

		memcpy(ptr, _str, str_size);											//포인터의 위치에 문자열을 복사
		ptr = ptr + str_size;													//포인터 문자열의 크기만큼 이동
		break;
	}

	if (send(_player->sock, _player->packetbuf, sizeof(size)+size, 0) == SOCKET_ERROR)	//위에서 만들어진 패킷을 해당 클라이언트에게 보냅니다
	{
		char err_str[BUFSIZE];
		sprintf(err_str, "StringSend() Error Protocol Number : %d", _protocol);									//에러시 나타날 오류문은 프로토콜의 고유숫자로 분별합니다
		err_display(err_str);															//오류시 err_display를 사용하고
		return false;																	//스레드 함수를 종료시키기위해 false를 반환합니다																
	}

	return true;																		//위의 send에서 제대로된 작업을 수행하였다면 여기에서 함수가 끝납니다
}

bool RecvFunction(_ClientInfo * _player)
{
	int size;
	int protocol;
	int retval;

	retval = recvn(_player->sock, (char*)&size, sizeof(size), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("g_value recv error()1");
		return false;
	}
	else if (retval == 0)
	{
		return 0;
	}

	retval = recvn(_player->sock, _player->packetbuf, size, 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("g_value recv error()2");
		return false;

	}
	else if (retval == 0)
	{
		return 0;
	}

	char * ptr = _player->packetbuf;
	memcpy(&protocol, ptr, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	switch (protocol)
	{
	case GVALUE:
		memcpy(&_player->gvalue, ptr, sizeof(_player->gvalue));
		break;
	}

	return true;
}

void AddThread(_ClientInfo * ptr)
{
	hThread[th_count] = CreateThread(NULL, 0, ProcessClient, ptr, 0, NULL);
	ptr->thread = hThread[th_count];
}

void RemoveThread(int index)
{
	for (int i = index ; i < th_count; i++)
	{
		hThread[i] = hThread[i + 1];
	}

	hThread[th_count] = NULL;
	th_count--;
}

void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
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
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
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
	_ClientInfo* ptr = new _ClientInfo;
	ZeroMemory(ptr, sizeof(_ClientInfo));
	ptr->sock = sock;
	memcpy(&(ptr->clientaddr), &clientaddr, sizeof(clientaddr));
	ptr->state = INTRO_SEND;
	ptr->gvalue = NODATA;

	ClientInfo[count] = ptr;
	printf("\nClient 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));
	count++;
	
	return ptr;
}

void RemoveClient(_ClientInfo* ptr)
{
	closesocket(ptr->sock);

	printf("\nClient 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(ptr->clientaddr.sin_addr),
		ntohs(ptr->clientaddr.sin_port));

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

}

bool MatchPartner(_ClientInfo* _ptr)
{
	for (int i = 0; i < count; i++)
	{
		if (_ptr != ClientInfo[i] && ClientInfo[i]->part == nullptr)
		{
			ClientInfo[i]->part = _ptr;
			_ptr->part = ClientInfo[i];
			return true;
		}
	}
	
	return false;
}

