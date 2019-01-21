#pragma comment(lib, "ws2_32")		//라이브러리 파일 "ws2_32"를 포함시킨다는 전처리문
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 512
#define MAXUSER 100
#define PLAYERCOUNT 20

enum RESULT { NODATA = 0, WIN, LOOSE, DRAW };
enum PROTOCOL { INTRO, INPUTDATA, GAME_RESULT };

struct User_Info		//유저의 정보를 저장할 구조체
{
	char * Result;
	int HandState;
};

struct sClientInfo		//클라이언트의 정보를 저장할 구조체
{
	SOCKET sock;				//소켓
	SOCKADDR_IN clientaddr;		//소켓애더
	User_Info info;				//유저의 정보
	sClientInfo * Partner;		//파트너 클라이언트
	HANDLE hThread;				//핸들
	int index;					//유저식별번호
	char buf[BUFSIZE + 1];		//버퍼
};

sClientInfo* ClientInfo[PLAYERCOUNT];		//클라이언트 정보 구조체 포인터 배열
int count = 0;								//카운트

sClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr);		//클라이언트 추가 함수
void RemoveClient(sClientInfo* ptr);								//클라이언트 삭제 함수
void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);
DWORD WINAPI ProcessClient(LPVOID arg);								//스레드가 배정될 함수				

int main(int argc, char* argv[])
{
	int retval;

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
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수		
	int addrlen;
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	while (1)
	{
		//accept()
		sClientInfo * NewPlayer;

		addrlen = sizeof(clientaddr);
		sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);									//새로운 유저 받음
		if (sock == INVALID_SOCKET)
		{
			err_display("accept()");
			continue;
		}

		NewPlayer = AddClient(sock, clientaddr);														//추가

		NewPlayer->hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)NewPlayer, 0, NULL);			//스레드 함수 생성

		if (NewPlayer->hThread == NULL)
		{
			RemoveClient(NewPlayer);																		//실패시 유저 삭제
		}

		else
		{
			CloseHandle(NewPlayer->hThread);																//성공시 스레드 함수 권한 포기
		}
	}

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
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


// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	//--------------------------------------------------------------------------------------------------------------
	//인트로 송신

	char* Intro = "0. 가위 1. 바위 2. 보\n";

	sClientInfo * Player = (sClientInfo*)arg;							//유저

	int retval = 0;

	PROTOCOL protocol = INTRO;
	int size = sizeof(PROTOCOL);
	int intro_size = strlen(Intro);
	size = size + (sizeof(intro_size) + intro_size);					//전체 사이즈

	char* ptr = Player->buf;
	memcpy(ptr, &size, sizeof(size));

	ptr = ptr + sizeof(size);
	memcpy(ptr, &protocol, sizeof(protocol));

	ptr = ptr + sizeof(protocol);
	memcpy(ptr, &intro_size, sizeof(intro_size));

	ptr = ptr + sizeof(intro_size);
	memcpy(ptr, Intro, intro_size);										//전체 사이즈 + 프로토콜 + 인트로 사이즈 + 인트로 문자열

	retval = send(Player->sock, Player->buf, sizeof(int) + size, 0);	//조합한 버퍼 클라이언트에게 송신
	if (retval == SOCKET_ERROR)
	{
		err_display("IntroSend()");

		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(Player->clientaddr.sin_addr), ntohs(Player->clientaddr.sin_port));

		RemoveClient(Player);											//send 실패시 클라이언트 삭제
		return NULL;
	}

	RESULT result = NODATA;

	//--------------------------------------------------------------------------------------------------------------
	//클라이언트의 선택값 받음

	size = 0;
	retval = recvn(Player->sock, (char*)&size, sizeof(int), 0);		//받을 데이터의 총크기

	if (retval == SOCKET_ERROR)
	{
		err_display("recv()");
		{
			RemoveClient(Player);
			return NULL;
		}
	}

	retval = recvn(Player->sock, Player->buf, size, 0);				//알아낸 크기만큼 데이터를 받음

	if (retval == SOCKET_ERROR)
	{
		err_display("recv()");
		{
			RemoveClient(Player);
			return NULL;
		}
	}

	ptr = Player->buf;

	//받은 데이터의 분해

	memcpy(&protocol, ptr, sizeof(protocol));					//프로토콜		

	if (protocol == INPUTDATA)
	{
		ptr = ptr + sizeof(protocol);
		memcpy(&Player->info.HandState, ptr, sizeof(Player->info.HandState));		//클라가 선택한 값
	}

	//--------------------------------------------------------------------------------------------------------------
	//결과 계산

	// 받은 데이터 출력
	printf("[TCP/%s:%d] %d번 플레이어 선택: %d\n", inet_ntoa(Player->clientaddr.sin_addr), ntohs(Player->clientaddr.sin_port), Player->index, Player->info.HandState);
	
	while (1)
	{
		if (Player->Partner != nullptr)						//파트너가 없으면 생길때까지 대기
		{
			if (Player->Partner->info.HandState != -1)		//아직 파트너가 내지않았을경우 대기
			{
				break;
			}
		}
	}

	//승부 계산
	Player->info.Result = (Player->info.HandState == Player->Partner->info.HandState) ? "Draw" : (((Player->info.HandState + 1) % 3 == Player->Partner->info.HandState) ? "You Lose!" : "You Win!");

	
	printf("플레이어%d : %s\n", Player->index, Player->info.Result);		//서버에서의 출력


	//-------------------------------------------------------------------------------------------------------------
	//결과 송신

	protocol = GAME_RESULT;							
	size = sizeof(protocol);
	int game_result_size = strlen(Player->info.Result);
	size = size + (sizeof(game_result_size) + game_result_size);						//전체 사이즈

	ptr = Player->buf;
	memcpy(ptr, &size, sizeof(size));

	ptr = ptr + sizeof(size);
	memcpy(ptr, &protocol, sizeof(protocol));

	ptr = ptr + sizeof(protocol);
	memcpy(ptr, &game_result_size, sizeof(game_result_size));

	ptr = ptr + sizeof(game_result_size);
	memcpy(ptr, Player->info.Result, game_result_size);					//전체 사이즈 + 프로토콜 + 결과 문자열 사이즈 + 결과 문자열

	retval = send(Player->sock, Player->buf, sizeof(int) + size, 0);	//조합한 버퍼 클라이언트에게 송신
	if (retval == SOCKET_ERROR)
	{
		err_display("game_result Send()");
	}

	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(Player->clientaddr.sin_addr), ntohs(Player->clientaddr.sin_port));
	RemoveClient(Player);

	return 0;
}


sClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr)
{
	sClientInfo* ptr = new sClientInfo;
	ZeroMemory(ptr, sizeof(sClientInfo));
	ptr->sock = sock;
	ptr->info.HandState = -1;

	for (int i = 0; i < count; i++)
	{
		if (ClientInfo[i]->Partner == nullptr)		//파트너가 없는 클라 발견시
		{
			ClientInfo[i]->Partner = ptr;			//현재 추가 클라 발견 클라와 파트너 연결
			ptr->Partner = ClientInfo[i];			//발견 클라 현재 추가 클라와 파트너 연결
			break;									//중복 연결이 되지 않도록 break
		}
	}

	memcpy(&(ptr->clientaddr), &clientaddr, sizeof(clientaddr));
	ClientInfo[count] = ptr;
	ClientInfo[count]->index = count;				//식별번호
	printf("\nClient 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));
	count++;
	return ptr;
}

void RemoveClient(sClientInfo* ptr)
{
	closesocket(ptr->sock);

	printf("\nClient 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(ptr->clientaddr.sin_addr), ntohs(ptr->clientaddr.sin_port));

	for (int i = 0; i < count; i++)
	{
		if (ClientInfo[i] == ptr)					
		{
			if (ptr->Partner != nullptr)				//삭제할 클라의 파트너가 널포인터가 아닌경우
			{
				ptr->Partner->Partner = nullptr;		//삭제할 클라의 파트너가 자신을 가르키는걸 널포인터를 가르키게 바꿔줌
			}
			
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