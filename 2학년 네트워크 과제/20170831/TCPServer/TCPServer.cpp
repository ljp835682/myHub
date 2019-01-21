#pragma comment(lib, "ws2_32")		//라이브러리 파일 "ws2_32"를 포함시킨다는 전처리문

									//lib는 정적 라이브러리 미리 이미 번역된 라이브러리를 포함시키는것
									//처음부터 포함되어있기에 만들어진 exe파일의 용량이 커지며 실행이 느리다

									//dll은 동적 라이브러리 필요할때 마다 번역
									//처음에는 쉽게 실행되고 만들어진 exe파일도 용량이 작지만 dll파일을 호출할때 느려진다
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)												//더이상 진행이 불가능해 프로그램을 종료해야할떄 호출하는 함수
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

/*
과제 err_display 가 어떤형식으로 돌아가는지 조사해올것
*/

// 소켓 함수 오류 출력
void err_display(char *msg)												//에러가 나도 진행이 가능할때 문자를 출력할 함수
{
	LPVOID lpMsgBuf;		//보이드 포인터
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);				//문자열 포인터로 변환하여 함수로 보내고 lpMsgBuf라는 보이드포인터에 어느 문자열의 주소를 저장시켜
	printf("[%s] %s", msg, (char *)lpMsgBuf);		//보이드포인터를 캐릭터 포인터로 명시적변환해서 출력
	LocalFree(lpMsgBuf);
}

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;									//윈도우 소켓 데이터
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)		//2.2버전의 윈도우소켓 dll라이브러리를 초기화시키는데 제대로되지 않을경우
		return 1;									//프로그램 종료

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);		//socket(IPv4 를 사용,TCP 프로토콜 사용,항상 0) 함수완료후 핸들(사용권한+식별번호)을 리턴
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");		//만약 사용할수 없는 소켓(안만들어질경우 등)일경우 err_quit함수 호출

	// bind()
	SOCKADDR_IN serveraddr;									
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;				//
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);	//주소정보 셋팅
	serveraddr.sin_port = htons(SERVERPORT);		//
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));	//바인딩된 소켓번호를 소켓에 할당한다
	if(retval == SOCKET_ERROR) err_quit("bind()");

	// listen()											//서버에서만 사용하는 함수	(서버에서 처음만들어진 소켓은 연결요청만을 받는 소켓이되며 이를 리슨소켓,서버소켓 이라 부른다)
	retval = listen(listen_sock, SOMAXCONN);			//리슨소켓이 가질수있는 최대크기
	if(retval == SOCKET_ERROR) err_quit("listen()");	//리슨소켓이 만들어지지 않을경우 에러

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE+1];

	while(1)
	{
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);		//accept -> 받는함수
																					//accept는 리슨소켓이 받아놓은 열결요청패킷을 가져와서 그 패킷이랑 연결할 소켓을 만들어서 그때야 포트가 만들어진다
																					//(SOCKADDR *)&clientaddr에는 이번에 만든 소켓에 연결된 클라이언트의 정보를 담아둔다
		if(client_sock == INVALID_SOCKET)											//리슨소켓이 연결요청 소켓을 받지못했을때 accept는 블럭(멈춤)된다
		{
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while(1)
		{
			// 데이터 받기															//recv -> 리시브함수
			retval = recv(client_sock, buf, BUFSIZE, 0);							//recv(데이터를 받을 소켓,버퍼,버퍼의 최대크기,무조건 0)

			if(retval == SOCKET_ERROR)												//성공실패 확인
			{
				err_display("recv()");
				break;
			}
			else if(retval == 0)													//만약 0일경우 클라우드 소켓이라는 뜻이므로 이 반복문을 종료
				break;

			// 받은 데이터 출력
			buf[retval] = '\0';														//리시브 함수로 받은 데이터는 마지막에 널이 없으므로 쓰래기값인지 아닌지 알수없기에 마지막에 널을 넣어준다
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),				//받은거 출력
				ntohs(clientaddr.sin_port), buf);

			// 데이터 보내기
			retval = send(client_sock, buf, retval, 0);								//받앗으니까 TCP프로토콜의 법칙에 의하여 다시 보내줘야하므로 센드 함수를 호출한다 
			if(retval == SOCKET_ERROR)
			{
				err_display("send()");
				break;
			}
		}

		// closesocket()
		closesocket(client_sock);													//closesocket함수는 소켓을 닫는용도의 함수
																					//대화가 끝나서 종료된 소켓을 닫는다
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",			//출력
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket(listen_sock);														//프로그램 종료전 리슨 소켓 닫기

	// 윈속 종료
	WSACleanup();																	//dll 정리
	return 0;
}