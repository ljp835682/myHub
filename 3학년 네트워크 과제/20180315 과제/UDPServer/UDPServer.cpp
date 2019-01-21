#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512
#define SIMBOL 0
#define ERRSIMBOL -1

#define PARK_ROOM "233.3.3.3"		
#define O_ROOM "233.3.3.4"		
#define KIM_ROOM "233.3.3.5"		

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

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");

	// 데이터 통신에 사용할 변수
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE+1] = "";
	bool flag = false;

	// 클라이언트와 데이터 통신
	while(1){
		// 데이터 받기
		addrlen = sizeof(clientaddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR *)&clientaddr, &addrlen);
		if(retval == SOCKET_ERROR){
			err_display("recvfrom()");
			continue;
		}

		switch (atoi(buf))
		{
		case 0:
			// 버퍼에 질문 입력
			strcpy_s(buf, "채팅방을 선택하세요\n1. 박용택\n2. 오지환\n3. 김현수\n 대답 : ");

			// 질문 보내기
			retval = sendto(sock, buf, strlen(buf), 0,
				(SOCKADDR *)&clientaddr, sizeof(clientaddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}

			flag = false;
			break;
		case 1:
			strcpy_s(buf, PARK_ROOM);	
			flag = true;
			break;
		case 2:	
			strcpy_s(buf, O_ROOM);		//맞는 주소를 입력시킴
			flag = true;
			break;
		case 3:
			strcpy_s(buf, KIM_ROOM);
			flag = true;
			break;
		default:
			strcpy_s(buf, "-1");		//그외일경우 에러심볼을 전송할것
			flag = true;
			break;
		}

		// 클라가 접속할 방의 주소를 보냄
		if (flag)
		{
			retval = sendto(sock, buf, strlen(buf), 0,
				(SOCKADDR *)&clientaddr, sizeof(clientaddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}
		}
		
		// 버퍼 초기화
		ZeroMemory(buf, BUFSIZE + 1);
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

//UDP를 사용한 채팅창 만들기
//서버와 클라이언트가 존재
//서버에게 클라이언트가 채팅요청 패킷을 보냄
//서버는 그 패킷으로 그 클라이언트의 주소를 알게되며
//그 클라이언트에게
//채팅방을 선택하세요
//1. 박용택방
//2. 오지환방
//3. 김현수방
//라는 메세지를 보내고
//그 클라이언트가 방을 선택한후
//서버는 224.0.0.0 ~ 235.255.255.255 의 주소를 기억하고있다가
//맞는 방의 주소를 클라에게 보내고
//클라는 그방에 들어간다