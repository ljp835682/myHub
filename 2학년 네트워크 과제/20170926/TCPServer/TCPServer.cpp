#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 512
#define MAXUSER 100

enum RESULT{ NODATA=0, IDERROR, PWERROR, LOGINSUCESS};
enum PROTOCOL{ INTRO, ID_PW_INFO, LOGIN_RESULT};

struct User_Info
{
	char ID[10];
	char PW[10];	
}LoginInfo[3] = { { "aaa", "111" }, { "bbb", "222" }, { "ccc", "333" } };

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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



int main(int argc, char* argv[])
{
	int retval;
	
	char* Intro = "���̵�� �н����带 �Է��ϼ���\n";	
	char* ID_error = "���� ���̵��Դϴ�\n";
	char* PW_error = "�н����尡 Ʋ�Ƚ��ϴ�.\n";
	char* Login_Success = "�α��ο� �����߽��ϴ�.\n";

	// ���� �ʱ�ȭ
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

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE+1];
	

	while(1)
	{
		// accept()		

		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET){
			err_display("accept()");
			continue;
		}
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", 
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		
		PROTOCOL protocol = INTRO;
		int size = sizeof(PROTOCOL);
		int intro_size = strlen(Intro);
		size = size + (sizeof(int)+intro_size);
		
		char* ptr = buf;
		memcpy(ptr, &size, sizeof(int));

		ptr = ptr + sizeof(int);
		memcpy(ptr, &protocol, sizeof(PROTOCOL));

		ptr = ptr + sizeof(PROTOCOL);
		memcpy(ptr, &intro_size, sizeof(int));

		ptr = ptr + sizeof(int);
		memcpy(ptr, Intro, intro_size);

		// Ŭ���̾�Ʈ�� ������ ���
		retval = send(client_sock, buf, sizeof(int)+size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			closesocket(client_sock);
			printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
			continue;
		}

		while(1)
		{

			RESULT result = NODATA;

			int size;
			retval = recvn(client_sock, (char*)&size, sizeof(int), 0);
			if(retval == SOCKET_ERROR){
				err_display("recv()");
				break;
			}
			else if(retval == 0)
				break;

			retval = recvn(client_sock, buf, size, 0);
			if (retval == SOCKET_ERROR){
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			PROTOCOL protocol;
			User_Info info;
			int idsize;
			int pwsize;
			ptr = buf;
			
			memcpy(&protocol, ptr, sizeof(PROTOCOL));

			if (protocol == ID_PW_INFO)
			{
				ptr = ptr + sizeof(PROTOCOL);
				memcpy(&idsize, ptr, sizeof(int));
				
				ptr = ptr+ sizeof(int);
				memcpy(info.ID, ptr, idsize);

				ptr = ptr + idsize;
				memcpy(&pwsize, ptr, sizeof(int));

				ptr = ptr + sizeof(int);
				memcpy(info.PW, ptr, pwsize);
			}

			// ���� ������ ���
			info.ID[idsize] = '\0';
			info.PW[pwsize] = '\0';
			printf("[TCP/%s:%d] ID: %s, PW:%s\n", inet_ntoa(clientaddr.sin_addr),
				ntohs(clientaddr.sin_port), info.ID, info.PW);

			int result_size;
			char result_temp[BUFSIZE];
			for (int i = 0; i<sizeof(LoginInfo) / sizeof(User_Info); i++)
			{
				if (!strcmp(info.ID, LoginInfo[i].ID))
				{
					if (!strcmp(info.PW, LoginInfo[i].PW))
					{
						result=LOGINSUCESS;	
						result_size = strlen(Login_Success);
						strcpy(result_temp, Login_Success);
					}
					else
					{
						result=PWERROR;
						result_size = strlen(PW_error);
						strcpy(result_temp, PW_error);
					}
					break;
				}
			}

			if (result == NODATA)
			{
				result=IDERROR;
				result_size = strlen(ID_error);
				strcpy(result_temp, ID_error);
			}
			

			ptr = buf;
			protocol = LOGIN_RESULT;
			size = sizeof(PROTOCOL)+sizeof(RESULT)+sizeof(int)+result_size;
			memcpy(ptr, &size, sizeof(int));

			ptr = ptr + sizeof(int);
			memcpy(ptr, &protocol, sizeof(PROTOCOL));

			ptr = ptr + sizeof(PROTOCOL);
			memcpy(ptr, &result, sizeof(RESULT));


			ptr = ptr + sizeof(RESULT);
			memcpy(ptr, &result_size, sizeof(int));


			ptr = ptr + sizeof(int);
			memcpy(ptr, result_temp, result_size);


			// ������ ������
			retval = send(client_sock, buf, sizeof(int)+size, 0);
			if(retval == SOCKET_ERROR){
				err_display("send()");
				break;
			}
		}

		// closesocket()
		closesocket(client_sock);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", 
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}

