#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"		
#define SERVERPORT 9000			
#define BUFSIZE    512
#define STRSIZE    128

enum RESULT{ NODATA = 0, IDERROR, PWERROR, LOGINSUCESS };
enum PROTOCOL{ INTRO, ID_PW_INFO, LOGIN_RESULT };

struct User_Info
{
	char ID[10];
	char PW[10];
};

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

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0)
	{
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

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);	
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);		
										
	serveraddr.sin_port = htons(SERVERPORT);			
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE + 1];
	int len;

	// 서버와 데이터 통신
	while (1)
	{
		int size;
		char* ptr;

		RESULT result = NODATA;

		//인트로 전체 크기 받음
		retval = recvn(sock, (char*)&size, sizeof(int), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		//인트로 받음
		retval = recvn(sock, buf, size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		PROTOCOL protocol;
		char intro[STRSIZE];		//인트로 저장 문자열
		int introsize;				//인트로 문자열 길이 저장 변수
		ptr = buf;

		//버퍼에 들어있는 데이터 전체길이 + 프로토콜 + 인트로의 길이 + 인트로 문자열
		memcpy(&protocol, ptr, sizeof(PROTOCOL));
		
		if (protocol == INTRO)
		{
			ptr = ptr + sizeof(PROTOCOL);
			memcpy(&introsize, ptr, sizeof(int));
			ptr = ptr + sizeof(int);
			memcpy(intro, ptr, introsize);
		}		//분해

		intro[introsize] = '\0';
		printf("%s\n", intro);
		//버퍼의 초기화
		ZeroMemory(buf, sizeof(buf));
		
		//---------------------------------------------------------

		char ID[STRSIZE];
		char PW[STRSIZE];

		printf("아이디 입력 : ");
		scanf("%s", ID);
		printf("비밀번호 입력 : ");
		scanf("%s", PW);

		protocol = ID_PW_INFO;
		size = sizeof(PROTOCOL);
		int idsize = strlen(ID) + 1;
		int pwsize = strlen(PW) + 1;

		//전체길이
		size = size + (sizeof(int)+idsize) + (sizeof(int)+pwsize);

		ptr = buf;
		memcpy(ptr, &size, sizeof(int));

		ptr = ptr + sizeof(int);
		memcpy(ptr, &protocol, sizeof(PROTOCOL));

		ptr = ptr + sizeof(PROTOCOL);
		memcpy(ptr, &idsize, sizeof(int));

		ptr = ptr + sizeof(int);
		memcpy(ptr, ID, idsize);

		ptr = ptr + sizeof(PROTOCOL);
		memcpy(ptr, &pwsize, sizeof(int));

		ptr = ptr + sizeof(int);
		memcpy(ptr, PW, pwsize);		//합성

		//전체길이 + 프로토콜 + 아이디 문자열 길이 + 아이디 문자열 + 비밀번호 문자열 길이 + 비밀번호 문자열

		//아이디와 비밀번호를 동시에 보냄
		retval = send(sock, buf, sizeof(int)+size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}

		//---------------------------------------------------------

		//결과 전체 크기 받음
		retval = recvn(sock, (char*)&size, sizeof(int), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		//결과 받음
		retval = recvn(sock, buf, size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		char str_result[STRSIZE];
		int str_result_size;

		ptr = buf;

		//전체길이 + 프로토콜 + 결과값 심볼 + 결과 문자열 길이 + 결과 문자열

		memcpy(&protocol, ptr, sizeof(PROTOCOL));

		if (protocol == LOGIN_RESULT)
		{
			ptr = ptr + sizeof(PROTOCOL);
			memcpy(&result, ptr, sizeof(RESULT));

			ptr = ptr + sizeof(RESULT);
			memcpy(&str_result_size, ptr, sizeof(int));

			ptr = ptr + sizeof(int);
			memcpy(str_result, ptr, str_result_size);
		}	//분해

		str_result[str_result_size] = '\0';
		printf("%s\n", str_result);		//출력
		ZeroMemory(buf, sizeof(buf));

		if (result == LOGINSUCESS)		//성공시 탈출
		{
			break;
		}
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
