#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512
#define MAXCLIENTNUMBER 2

struct sClientInfo
{
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	char * Result;
	int HandState;
};

int Count = 0;
sClientInfo * ClientInfo[MAXCLIENTNUMBER] = {nullptr,nullptr};

sClientInfo * AddClientInfo(SOCKET _sock, SOCKADDR_IN _clientaddr)
{
	sClientInfo * NewClient = new sClientInfo();
	ZeroMemory(NewClient, sizeof(sClientInfo));
	NewClient->sock = _sock;
	NewClient->clientaddr = _clientaddr;
	NewClient->HandState = -1;
	memcpy(&(NewClient->clientaddr), &_clientaddr, sizeof(_clientaddr));
	ClientInfo[Count] = NewClient;
	Count++;
	return NewClient;
}

void CloseClient(sClientInfo * _Client)
{
	closesocket(_Client->sock);

	for (int i = 0; i < Count; i++)
	{
		if (ClientInfo[i] == _Client)
		{
			delete _Client;
			int j;
			for (j = i; j < Count - 1; j++)
			{
				ClientInfo[j] = ClientInfo[j + 1];
			}
			ClientInfo[j] = nullptr;
			break;
		}
	}

	Count--;
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

	// ������ ��ſ� ����� ����
	int addrlen;
	SOCKET temp_sock;
	SOCKADDR_IN clientaddr;

	char * intro = "Game Start!";
	char buf[BUFSIZE+1];

	while (1)
	{
		//accept(),send() Ŭ�� ������ �޼����� ���� ���� ���� Ŭ�� ����
		while (1)
		{
			addrlen = sizeof(clientaddr);
			temp_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
			if (temp_sock == INVALID_SOCKET)
			{
				err_display("accept()");
				continue;
			}

			sClientInfo* NewClient = AddClientInfo(temp_sock, clientaddr);

			if (send(NewClient->sock, (char*)intro, strlen(intro), 0) == SOCKET_ERROR)
			{
				err_display("IntroSend()");
				CloseClient(NewClient);
			}

			if (Count == MAXCLIENTNUMBER)
			{
				break;
			}
		}

		//�����Ǵܿ� �÷���
		bool cl_errflag = false;

		// recvn() �� Ŭ���̾�Ʈ�� ������ �޾ƿ��� �κ� int���� �ð��� �˰������Ƿ� recvn�� �����
		for (int i = 0; i < MAXCLIENTNUMBER; i++)
		{
			if (ClientInfo[i] != nullptr)
			{
				if (ClientInfo[i]->HandState == -1)
				{
					retval = recvn(ClientInfo[i]->sock, (char*)&ClientInfo[i]->HandState, sizeof(int), 0);
				}

				if (retval == SOCKET_ERROR)
				{
					err_display("recv()");
					CloseClient(ClientInfo[i]);
					i--;
					cl_errflag = true;
				}

				retval = 0;
			}
		}

		if (cl_errflag)
		{
			continue;
		}

		//���� ���ڿ� ���� (���������� ���� ��Ģ)
		ClientInfo[0]->Result = (ClientInfo[0]->HandState == ClientInfo[1]->HandState) ? "Draw" : (((ClientInfo[0]->HandState + 1) % 3 == ClientInfo[1]->HandState) ? "You Lose!" : "You Win!");
		ClientInfo[1]->Result = (ClientInfo[1]->HandState == ClientInfo[0]->HandState) ? "Draw" : (((ClientInfo[1]->HandState + 1) % 3 == ClientInfo[0]->HandState) ? "You Lose!" : "You Win!");		//���� ���ڿ� ����			


		// send() �� Ŭ�󿡰� ���� �´� �޼����� ����
		for (int i = 0; i < MAXCLIENTNUMBER; i++)
		{
			retval = send(ClientInfo[0]->sock, ClientInfo[0]->Result, strlen(ClientInfo[0]->Result), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("send()");
			}

			CloseClient(ClientInfo[0]);
		}
	}

	// closesocket()
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();															
	return 0;
}

//Ŭ�� �θ��� �پ �������������� ������ ������0 ������1 ���� 2 ���� �޼����� ������
//�������� ����
