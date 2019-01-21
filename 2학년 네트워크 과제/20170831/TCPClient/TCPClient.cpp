#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"		//루프팩 (자기자신으로 돌아오는 주소)
#define SERVERPORT 9000				//서버 포트
#define BUFSIZE    512

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

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while(left > 0)
	{
		received = recv(s, ptr, left, flags);	//소켓에서 데이터를 수신하는 함수
		if(received == SOCKET_ERROR)			//에러일경우
			return SOCKET_ERROR;				
		else if(received == 0)					
			break;
		left -= received;						//받은만큼 뺌
		ptr += received;						//수신할 위치의 포인터 데이터만큼 옮김
	}

	return (len - left);						//목표량  len에서 남은 용량을 뺌
}												//정확히 몇 바이트를 받아야할지 알때만 사용

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);			//소켓 생성 부분
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);		//서버 아이피
	serveraddr.sin_port = htons(SERVERPORT);				//서버 포트
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
															//위에서 만든 소켓으로 serveraddr에 저장된 서버에게 연결을 요청
															//bind의 역활을 connect가 해주며 OS가 사설 포트번호중 사용중이 아닌것 하나를 자동으로 할당함
															//connect는 연결 요청 패킷을 보낼뿐
	if(retval == SOCKET_ERROR) err_quit("connect()");		

	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE+1];
	int len;

	// 서버와 데이터 통신
	while(1){
		// 데이터 입력
		printf("\n[보낼 데이터] ");
		if(fgets(buf, BUFSIZE+1, stdin) == NULL)		//fget은 file에서 문자어를 읽어서 버퍼에세 넢는 함수
			break;

		// '\n' 문자 제거
		len = strlen(buf);
		if(buf[len-1] == '\n')
			buf[len-1] = '\0';							//엔터입력시 closesocket으로 이동
		if(strlen(buf) == 0)
			break;

		// 데이터 보내기
		retval = send(sock, buf, strlen(buf), 0);		//버퍼에 들어온 길이만큼만 상대 서버 소켓에 보냄
		if(retval == SOCKET_ERROR){
			err_display("send()");
			break;
		}
		printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);	//소켓의 송신 버퍼에 쓰기만함, 아직 연결성립은 아님
	
		// 데이터 받기
		retval = recvn(sock, buf, retval, 0);			//send로 보낸만큼 받아내기위해 retval을 사용
		if(retval == SOCKET_ERROR){
			err_display("recv()");
			break;
		}
		else if(retval == 0)
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
		printf("[받은 데이터] %s\n", buf);
	}

	// closesocket()
	closesocket(sock);				

	// 윈속 종료
	WSACleanup();
	return 0;
}