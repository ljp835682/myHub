#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512
#define ID "a"
#define PW "1"

enum PROTOCOL { INTRO, LOGIN, RESULT };
enum STATE { NON_LOGIN, DO_LOGIN, END_LOGIN, EXIT };

// ���� ���� ������ ���� ����ü�� ����
struct SOCKETINFO
{
	SOCKET sock;
	char buf[BUFSIZE+1];
	int size;
	int recvbytes;
	int sendbytes;
	STATE state;
	char packetbuf[BUFSIZE];
	bool loginflag;
};

int nTotalSockets = 0;
SOCKETINFO *SocketInfoArray[FD_SETSIZE];

// ���� ���� �Լ�
BOOL AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int nIndex);

void PackPacket(char * packetbuf, PROTOCOL protocol, char * str1, int & size);
void GetProtocol(char * packetbuf, PROTOCOL & protocol);
void UnPackPacket(char * packetbuf, char * str1, char * str2);

// ���� ��� �Լ�
void err_quit(char *msg);
void err_display(char *msg);

int recvn(SOCKET s, char *buf, int len, int flags);

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
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

	// �ͺ��ŷ �������� ��ȯ
	u_long on = 1;
	retval = ioctlsocket(listen_sock, FIONBIO, &on);
	if(retval == SOCKET_ERROR) err_display("ioctlsocket()");

	// ������ ��ſ� ����� ����
	FD_SET rset, wset;
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen, i;

	while(1){
		// ���� �� �ʱ�ȭ
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(listen_sock, &rset);
	
		for(i=0; i<nTotalSockets; i++)
		{
			//��� ���� �߰�
			FD_SET(SocketInfoArray[i]->sock, &rset);

			//��Ȳ�� �´� ���ϸ� �߰�
			if (SocketInfoArray[i]->state == NON_LOGIN || SocketInfoArray[i]->state == END_LOGIN)
			{
				FD_SET(SocketInfoArray[i]->sock, &wset);
			}
		}

		// select()
		retval = select(0, &rset, &wset, NULL, NULL);
		if(retval == SOCKET_ERROR) err_quit("select()");

		// ���� �� �˻�(1): Ŭ���̾�Ʈ ���� ����
		if(FD_ISSET(listen_sock, &rset)){
			addrlen = sizeof(clientaddr);
			client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
			if(client_sock == INVALID_SOCKET){
				err_display("accept()");
			}
			else{
				printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
					inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
				// ���� ���� �߰�
				AddSocketInfo(client_sock);
			}
		}

		// ���� �� �˻�(2): ������ ���
		for(i=0; i<nTotalSockets; i++)
		{
			SOCKETINFO *ptr = SocketInfoArray[i];
			if (FD_ISSET(ptr->sock, &rset))
			{
				//DO_LOGIN �ϰ�쿡��
				if (ptr->state == DO_LOGIN)
				{
					if (ptr->size == 0)
					{
						//������ �޾ƿ�
						retval = recvn(ptr->sock, (char*)&ptr->size, sizeof(ptr->size), 0);
						if (retval == SOCKET_ERROR)
						{
							err_display("RecvError1()");
							break;
						}
					}

					else
					{
						char * workptr = ptr->packetbuf + ptr->recvbytes;

						//ptr->size - ptr->recvbytes �� �����κ��� �޾ƿ�
						retval = recv(ptr->sock, workptr, ptr->size - ptr->recvbytes, 0);
						if (retval == SOCKET_ERROR)
						{
							err_display("recv()");
							RemoveSocketInfo(i);
							continue;
						}

						else if (retval == 0)
						{
							RemoveSocketInfo(i);
							continue;
						}

						ptr->recvbytes += retval;

						if (ptr->recvbytes == ptr->size)
						{
							ptr->size = 0;
							ptr->recvbytes = 0;
							ptr->state = END_LOGIN;

							char buf1[BUFSIZE], buf2[BUFSIZE];
							UnPackPacket(ptr->packetbuf, buf1, buf2);

							if (strcmp(buf1, ID) == 0 && strcmp(buf2, PW) == 0)
							{
								ptr->loginflag = true;
							}
						}
					}
				}
			}

			if(FD_ISSET(ptr->sock, &wset))
			{
				int size = 0;

				switch (ptr->state)
				{
				case NON_LOGIN:
					//��ŷ
					PackPacket(ptr->packetbuf, INTRO, "ID,PW �Է� : \n", size);
					retval = send(ptr->sock, ptr->packetbuf + ptr->sendbytes, size - ptr->sendbytes, 0);
					if (retval == SOCKET_ERROR)
					{
						err_display("send()");
						RemoveSocketInfo(i);
						continue;
					}
					ptr->state = DO_LOGIN;
					break;

				case END_LOGIN:
					//���� ���� ����
					if (ptr->loginflag)
					{
						PackPacket(ptr->packetbuf, RESULT, "�α��� ����\n", size);
					}

					else
					{
						PackPacket(ptr->packetbuf, RESULT, "�α��� ����\n", size);
					}
					
					//����
					retval = send(ptr->sock, ptr->packetbuf + ptr->sendbytes, size - ptr->sendbytes, 0);
					if (retval == SOCKET_ERROR)
					{
						err_display("send()");
						RemoveSocketInfo(i);
						continue;
					}
					ptr->state = EXIT;
					break;
				}

				ptr->sendbytes += retval;
				if(size == ptr->sendbytes)
				{
					ptr->sendbytes = 0;
				}
			}
		}
	}

	// ���� ����
	WSACleanup();
	return 0;
}

// ���� ���� �߰�
BOOL AddSocketInfo(SOCKET sock)
{
	if(nTotalSockets >= FD_SETSIZE){
		printf("[����] ���� ������ �߰��� �� �����ϴ�!\n");
		return FALSE;
	}

	SOCKETINFO *ptr = new SOCKETINFO;
	if(ptr == NULL){
		printf("[����] �޸𸮰� �����մϴ�!\n");
		return FALSE;
	}

	ptr->sock = sock;
	ptr->sendbytes = 0;
	ptr->recvbytes = 0;
	ptr->size = 0;
	ptr->state = NON_LOGIN;
	ptr->loginflag = false;
	SocketInfoArray[nTotalSockets++] = ptr;

	return TRUE;
}

// ���� ���� ����
void RemoveSocketInfo(int nIndex)
{
	SOCKETINFO *ptr = SocketInfoArray[nIndex];

	// Ŭ���̾�Ʈ ���� ���
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR *)&clientaddr, &addrlen);
	printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", 
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	closesocket(ptr->sock);
	delete ptr;

	if(nIndex != (nTotalSockets-1))
		SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets-1];

	--nTotalSockets;
}

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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

void PackPacket(char * packetbuf, PROTOCOL protocol, char * str1, int & size)
{
	char * ptr = packetbuf;
	int strsize1 = strlen(str1);
	size = sizeof(protocol) + sizeof(strsize1) + strsize1;

	memcpy(ptr, &size, sizeof(size));
	ptr = ptr + sizeof(size);

	memcpy(ptr, &protocol, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	memcpy(ptr, &strsize1, sizeof(strsize1));
	ptr = ptr + sizeof(strsize1);

	memcpy(ptr, str1, strsize1);
	ptr = ptr + strsize1;

	size = size + sizeof(size);
}

void GetProtocol(char * packetbuf, PROTOCOL & protocol)
{
	memcpy(&protocol, packetbuf, sizeof(PROTOCOL));
}

void UnPackPacket(char * packetbuf, char * str1, char * str2)
{
	char * ptr = packetbuf + sizeof(PROTOCOL);
	int strsize1 = 0;
	int strsize2 = 0;

	memcpy(&strsize1, ptr, sizeof(strsize1));
	ptr += sizeof(strsize1);

	memcpy(str1, ptr, strsize1);
	ptr += strsize1;

	memcpy(&strsize2, ptr, sizeof(strsize2));
	ptr += sizeof(strsize2);

	memcpy(str2, ptr, strsize2);
	ptr += strsize2;

	str1[strsize1] = '\0';
	str2[strsize2] = '\0';
}