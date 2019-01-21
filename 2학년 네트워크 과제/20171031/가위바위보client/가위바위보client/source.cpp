/*
 * echo_client_win.c
 * Written by SW. YOON
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUFSIZE 512

enum PROTOCOL{ INTRO, GVALUE, RESULT };

void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);



int main(int argc, char **argv)
{
  WSADATA wsaData;

  int result;
  SOCKET hSocket;
  SOCKADDR_IN servAddr;
  char buf[BUFSIZE];
  int size;
  int gvalue;
  int strsize;
  char str[BUFSIZE];
  PROTOCOL protocol;
 
  if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */
		err_quit("WSAStartup() error!"); 

  hSocket=socket(PF_INET, SOCK_STREAM, 0);   
  if(hSocket == INVALID_SOCKET)
    err_quit("socket() error");

  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family=AF_INET;
  servAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
  servAddr.sin_port=htons(9000);

  if(connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr))==SOCKET_ERROR)
    err_quit("connect() error!");

  bool endflag = false;

  while (1)
  {
	  result = recvn(hSocket, (char*)&size, sizeof(size), 0);
	  if (result == SOCKET_ERROR || result == 0)
	  {
		  err_display("recv error()1");
		  return -1;
	  }

	  result = recvn(hSocket, buf, size, 0);
	  if (result == SOCKET_ERROR || result == 0)
	  {
		  err_display("recv error()2");
		  return -1;
	  }

	  char* ptr = buf;
	  memcpy(&protocol, ptr, sizeof(protocol));
	  ptr = ptr + sizeof(protocol);

	  switch (protocol)
	  {
	  case INTRO:
		  memcpy(&strsize, ptr, sizeof(strsize));
		  ptr = ptr + sizeof(strsize);

		  memcpy(str, ptr, strsize);

		  str[strsize] = '\0';
		  printf("%s\n", str);

		  scanf("%d", &gvalue);

		  ptr = buf;
		  protocol = GVALUE;
		  size = sizeof(protocol)+sizeof(gvalue);

		  memcpy(ptr, &size, sizeof(size));
		  ptr = ptr + sizeof(size);

		  memcpy(ptr, &protocol, sizeof(protocol));
		  ptr = ptr + sizeof(protocol);

		  memcpy(ptr, &gvalue, sizeof(gvalue));

		  result = send(hSocket, buf, sizeof(size)+size, 0); /* 메시지 전송 */
		  if (result == SOCKET_ERROR)
		  {
			  err_display("GameValue send error()");
			  return -1;
		  }

		  break;
	  case RESULT:
		 
		  memcpy(&strsize, ptr, sizeof(strsize));
		  ptr = ptr + sizeof(strsize);

		  memcpy(str, ptr, strsize);

		  str[strsize] = '\0';
		  printf("%s\n", str);
		
		  endflag = true;
	  }

	  if (endflag)
	  {
		  break;
	  }

  }  

  closesocket(hSocket);
  WSACleanup();
  return 0;
}

void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
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

	while(left > 0){
		received = recv(s, ptr, left, flags);
		if(received == SOCKET_ERROR) 
			return SOCKET_ERROR;
		else if(received == 0) 
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
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}