#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000					//서버하고 연결할 포트
#define BUFSIZE    512
#define REMOTEPORT  9003				//클라끼리 연결할 포트
#define LOCALPORT   9003				//클라끼리 연결할 포트
#define SIMBOL 0
#define ERRSIMBOL -1

DWORD WINAPI RecvThread(LPVOID arg);	//읽기 스레드

char address[100] = "";					//클라이언트가 들어갈 멀티캐스팅용 주소를 저장할 공간

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
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// 소켓 주소 구조체 초기화
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);

	// 데이터 통신에 사용할 변수
	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[BUFSIZE+1] = "0";
	int len;
	int choice;

	// 서버와 데이터 통신
	while(1)
	{
		// 심볼 보내기
		retval = sendto(sock, buf, strlen(buf), 0,
			(SOCKADDR *)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}

		// 질문 받기
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}

		// 널문자 삽입
		buf[retval] = '\0';

		// 질문 출력	
		printf("%s", buf);
		ZeroMemory(buf, BUFSIZE + 1);

		// 질문 작성겸 심볼 입력 불가하도록 지정
		while (1)
		{
			scanf("%d", &choice);
			if (choice != SIMBOL && choice != ERRSIMBOL)
			{
				break;
			}

			else
			{
				printf("ERR!!\n");
			}
		}
		

		sprintf(buf, "%d", choice);

		// 대답 보내기
		retval = sendto(sock, buf, strlen(buf), 0,
			(SOCKADDR *)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}

		// 받은 주소 확인하기
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}

		// 전역 주소 저장
		strcpy_s(address, buf);
		ZeroMemory(buf, BUFSIZE + 1);

		// 멀티캐스트 그룹 가입 실패시
		if (atoi(address) == ERRSIMBOL)
		{
			printf("ERR!!\n");
		}

		// 멀티캐스트 그룹 사입 성공시
		else
		{
			// 리시브 스레드 생성
			HANDLE hThread = CreateThread(NULL, 0, RecvThread, NULL, 0, NULL);
			if (hThread == NULL) return 1;
			CloseHandle(hThread);

			// 멀티캐스트 Time-to-live 설정
			int ttl = 2;
			retval = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
				(char *)&ttl, sizeof(ttl));

			if (retval == SOCKET_ERROR) err_quit("setsockopt()");

			// 소켓 주소 구조체 초기화
			SOCKADDR_IN remoteaddr;
			ZeroMemory(&remoteaddr, sizeof(remoteaddr));
			remoteaddr.sin_family = AF_INET;
			remoteaddr.sin_addr.s_addr = inet_addr(address);
			remoteaddr.sin_port = htons(REMOTEPORT);

			// 버퍼 초기화
			ZeroMemory(buf, BUFSIZE + 1);

			// 무한 입력을 위한 반복문
			while (1)
			{
				// 데이터 입력
				printf("\n[보낼 데이터] ");
				scanf("%s", buf);

				// '\n' 문자 제거
				len = strlen(buf);
				if (buf[len - 1] == '\n')
					buf[len - 1] = '\0';
				if (strlen(buf) == 0)
					break;

				// 데이터 보내기
				retval = sendto(sock, buf, strlen(buf), 0,
					(SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
				if (retval == SOCKET_ERROR) {
					err_display("sendto()");
					continue;
				}
			}
		}
	}

	// 소켓 닫음
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

DWORD WINAPI RecvThread(LPVOID arg)
{
	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// SO_REUSEADDR 옵션 설정
	BOOL optval = TRUE;
	int retval = setsockopt(sock, SOL_SOCKET,
		SO_REUSEADDR, (char *)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// 바인드
	SOCKADDR_IN localaddr;
	ZeroMemory(&localaddr, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(LOCALPORT);
	retval = bind(sock, (SOCKADDR *)&localaddr, sizeof(localaddr));
	if (retval == SOCKET_ERROR)
	{
		err_quit("bind()");
	}

	// 멀티캐스트 그룹 가입
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(address);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	retval = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// 데이터 통신에 사용할 변수
	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	// 멀티캐스트 데이터 받기
	while (1) {
		// 데이터 받기
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("[UDP/%s:%d] %s\n", inet_ntoa(peeraddr.sin_addr),
			ntohs(peeraddr.sin_port), buf);
	}

	// 멀티캐스트 그룹 탈퇴
	retval = setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");


	return 0;
}