#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
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
	SOCKET client_sock[2];
	SOCKADDR_IN clientaddr[2];
	int addrlen[2];					

	char buf[BUFSIZE+1] = "Game Start!";
	bool flag = false;

	while(1)
	{
		bool Powerflag = false;

		//오류시 반복문 탈출용
		if (flag)
		{
			break;
		}

		// accept()	클라를 두개받음
		for (int i = 0; i < 2; i++)
		{
			addrlen[i] = sizeof(clientaddr[i]);
			client_sock[i] = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen[i]);
			if (client_sock[i] == INVALID_SOCKET)
			{
				err_display("accept()");
				flag = true;
				break;
			}
		}

		// send() 두개의 클라에게 같은 메세지를 보냄
		for (int i = 0; i < 2; i++)
		{
			retval = send(client_sock[i], buf, strlen(buf), 0);

			if (retval == SOCKET_ERROR)
			{
				err_display("send()");
				flag = true;
				break;
			}
		}
											
		while(1)
		{
			//오류시 반복문 탈출용
			if (flag)
			{
				break;
			}

			int HandState[2];	//두 클라이언트가 낸것을 저장할 공간

			// recvn() 두 클라이언트가 낸것을 받아오는 부분 int형이 올것을 알고있으므로 recvn을 사용함
			for (int i = 0; i < 2; i++)
			{
				retval = recvn(client_sock[i], (char*)&HandState[i], sizeof(int), 0);

				if (retval == SOCKET_ERROR)
				{
					err_display("recv()");
					flag = true;
					break;
				}
				else if (retval == 0)
					break;

			}

			char * User_result[2];		//임시 저장용 공간
			User_result[0] = (HandState[0] == HandState[1]) ? "Draw" : (((HandState[0] + 1) % 3 == HandState[1]) ? "You Lose!" : "You Win!");
			User_result[1] = (HandState[1] == HandState[0]) ? "Draw" : (((HandState[1] + 1) % 3 == HandState[0]) ? "You Lose!" : "You Win!");		//보낼 문자열 지정			

			// send() 두 클라에게 각각 자신에게 맞는 메세지를 보냄
			for (int i = 0; i < 2; i++)
			{
				retval = send(client_sock[i], User_result[i], strlen(User_result[i]), 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("send()");
					flag = true;
					break;
				}

				//정상적으로 클라이언트가 종료되면 서버는 이곳을 지나게댐
				else
				{
					Powerflag = true;
				}
			}

			//새로운 클라를 받기위해 반복문을 나감
			if (Powerflag)
			{
				break;
			}

		}

		// closesocket()
		for (int i = 0; i < 2; i++)
		{
			closesocket(client_sock[i]);
		}
	}

	// closesocket()
	closesocket(listen_sock);														
	// 윈속 종료
	WSACleanup();															
	return 0;
}

//클라가 두명이 붙어서 가위바위보를함 서버는 바위는0 가위는1 보는 2 같은 메세지를 보내줌
//단판으로 끝냄
