#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 512
#define MAXUSER 100
#define PLAYERCOUNT 20

enum RESULT{ NODATA=0, IDERROR, PWERROR, LOGINSUCESS};
enum PROTOCOL{ INTRO, ID_PW_INFO, LOGIN_RESULT};

struct User_Info		//유저의 정보를 저장할 구조체
{
	char ID[10];
	char PW[10];	
};

User_Info LoginInfo[3] = { { "aaa", "111" }, { "bbb", "222" }, { "ccc", "333" } };	//미리 지정해둔 3개의 유저정보

struct sClientInfo		//클라이언트의 정보를 저장할 구조체
{
	SOCKET sock;				//소켓
	SOCKADDR_IN clientaddr;		//소켓애더
	User_Info info;				//유저의 정보
	HANDLE hThread;				//핸들
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
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");	
	
	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");
	
	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수		
	int addrlen;
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	
	while (1)
	{
		// accept()		
		addrlen = sizeof(clientaddr);
		sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (sock == INVALID_SOCKET)
		{
			err_display("accept()");
			continue;
		}

		sClientInfo* ptr = AddClient(sock, clientaddr);		//accept가 성공하면 접속한 클라정보를 저장

		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(ptr->clientaddr.sin_addr), ntohs(ptr->clientaddr.sin_port));

		ptr->hThread = CreateThread(NULL, 0, ProcessClient,			//새로운 스레드 생성하고
			(LPVOID)ptr, 0, NULL);									//그 스레드를 ProcessClient 에 할당

		if (ptr->hThread == NULL)					//만약 스레드 만드는게 실패하면
		{
			RemoveClient(ptr);						//클라정보 삭제
		}

		else
		{
			CloseHandle(ptr->hThread);				//소켓만드는데 성공하면 알아서 처리하라고 권리를 포기해버림
		}
	}

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}


// 소켓 함수 오류 출력 후 종료
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
	sClientInfo * NewClient = (sClientInfo*)arg;

	char* Intro = "아이디와 패스워드를 입력하세요\n";
	char* ID_error = "없는 아이디입니다\n";
	char* PW_error = "패스워드가 틀렸습니다.\n";
	char* Login_Success = "로그인에 성공했습니다.\n";

	int retval;
	int addrlen;

	// 클라이언트 정보 얻기
	addrlen = sizeof(NewClient->clientaddr);
	//getpeername(NowUser->sock, (SOCKADDR *)&NowUser->clientaddr, &addrlen);		//이 소켓에 연결된 상대방 소켓의 주소를 알아내는 함수
																					//필요없는 구문
	PROTOCOL protocol = INTRO;					
	int size = sizeof(PROTOCOL);
	int intro_size = strlen(Intro);
	size = size + (sizeof(int) + intro_size);

	char* ptr = NewClient->buf;
	memcpy(ptr, &size, sizeof(int));

	ptr = ptr + sizeof(int);
	memcpy(ptr, &protocol, sizeof(PROTOCOL));

	ptr = ptr + sizeof(PROTOCOL);
	memcpy(ptr, &intro_size, sizeof(int));

	ptr = ptr + sizeof(int);
	memcpy(ptr, Intro, intro_size);			//클라이언트에게 보낼 버퍼 조합
											//전체길이 + 프로토콜 + 인트로문자열의 길이 + 인트로문자열

	// 클라이언트와 데이터 통신
	retval = send(NewClient->sock, NewClient->buf, sizeof(int) + size, 0);	//조합한 버퍼 클라이언트에게 송신
	if (retval == SOCKET_ERROR)
	{
		err_display("send()");

		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(NewClient->clientaddr.sin_addr), ntohs(NewClient->clientaddr.sin_port));

		RemoveClient(NewClient);		//send 실패시 클라이언트 삭제
		return NULL;					//그후 함수 종료
	}

	while (1)
	{
		RESULT result = NODATA;		

		int size;
		retval = recvn(NewClient->sock, (char*)&size, sizeof(int), 0);		//받을 데이터의 총크기만 받아봄
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		retval = recvn(NewClient->sock, NewClient->buf, size, 0);			//위에서 알아낸 크기만큼 데이터를 받아옴
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		PROTOCOL protocol;
		int idsize;
		int pwsize;
		ptr = NewClient->buf;

		//받은 데이터의 분해

		memcpy(&protocol, ptr, sizeof(PROTOCOL));					//프로토콜		

		if (protocol == ID_PW_INFO)
		{
			ptr = ptr + sizeof(PROTOCOL);
			memcpy(&idsize, ptr, sizeof(int));						//아이디의 길이

			ptr = ptr + sizeof(int);
			memcpy(NewClient->info.ID, ptr, idsize);				//아이디 문자열

			ptr = ptr + idsize;
			memcpy(&pwsize, ptr, sizeof(int));						//패스워드의 길이

			ptr = ptr + sizeof(int);
			memcpy(NewClient->info.PW, ptr, pwsize);				//패스워드 문자열
		}								

		// 받은 데이터 출력
		NewClient->info.ID[idsize] = '\0';
		NewClient->info.PW[pwsize] = '\0';
		printf("[TCP/%s:%d] ID: %s, PW:%s\n", inet_ntoa(NewClient->clientaddr.sin_addr),
			ntohs(NewClient->clientaddr.sin_port), NewClient->info.ID, NewClient->info.PW);
																	
		int result_size;
		char result_temp[BUFSIZE];
		for (int i = 0; i < sizeof(LoginInfo) / sizeof(User_Info); i++)		//저장된 유저정보량 만큼 반복
		{
			if (!strcmp(NewClient->info.ID, LoginInfo[i].ID))
			{
				if (!strcmp(NewClient->info.PW, LoginInfo[i].PW))
				{
					result = LOGINSUCESS;						//로그인 성공
					result_size = strlen(Login_Success);
					strcpy(result_temp, Login_Success);
				}
				else
				{
					result = PWERROR;							//패스워드 미스매칭
					result_size = strlen(PW_error);
					strcpy(result_temp, PW_error);
				}
				break;
			}
		}

		if (result == NODATA)
		{
			result = IDERROR;									//아이디 비밀번호 미스매칭				
			result_size = strlen(ID_error);
			strcpy(result_temp, ID_error);
		}


		ptr = NewClient->buf;									//초기화
		protocol = LOGIN_RESULT;								//로그인 결과 문자열을 버퍼에 조합
		size = sizeof(PROTOCOL) + sizeof(RESULT) + sizeof(int) + result_size;		//총크기
		memcpy(ptr, &size, sizeof(int));

		ptr = ptr + sizeof(int);
		memcpy(ptr, &protocol, sizeof(PROTOCOL));			

		ptr = ptr + sizeof(PROTOCOL);
		memcpy(ptr, &result, sizeof(RESULT));


		ptr = ptr + sizeof(RESULT);
		memcpy(ptr, &result_size, sizeof(int));


		ptr = ptr + sizeof(int);
		memcpy(ptr, result_temp, result_size);			//클라이언트에게 보낼 버퍼 조합
														//전체길이 + 프로토콜 + 결과문자열의 길이 + 결과문자열


		// 데이터 보내기
		retval = send(NewClient->sock, NewClient->buf, sizeof(int) + size, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	// closesocket()
	closesocket(NewClient->sock);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(NewClient->clientaddr.sin_addr), ntohs(NewClient->clientaddr.sin_port));

	RemoveClient(NewClient);
	return 0;
}


sClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr)
{
	sClientInfo* ptr = new sClientInfo;
	ZeroMemory(ptr, sizeof(sClientInfo));
	ptr->sock = sock;
	strcpy_s(ptr->info.ID, strlen("") + 1, "");
	strcpy_s(ptr->info.PW, strlen("") + 1, "");
	memcpy(&(ptr->clientaddr), &clientaddr, sizeof(clientaddr));
	ClientInfo[count] = ptr;
	printf("\nClient 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));
	count++;
	return ptr;
}

void RemoveClient(sClientInfo* ptr)
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