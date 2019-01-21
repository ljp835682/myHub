#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 4096
#define FILENAMESIZE 256
#define MAXCONCLIENT 100
#define FILEEXIST -1

enum PROTOCOL{ INTRO, NOW_SIZE, DATA, DONE};

/*
1. INTRO : _DataPacket ����ü�� �ۼ����ϴ� ��������
2. NOW_SIZE : nowtranssize �� �ۼ����ϴ� ��������
3. DATA : nowtranssize�� ������ ���ڿ� �� �ۼ����ϴ� ��������
4. DONE : �������ݸ��� �ۼ����ϴ� ��������
*/

struct _ClientInfo
{
	SOCKET sock;
	SOCKADDR_IN addr;
	char filename[FILENAMESIZE];
	int filesize;
	int transsize;
};

struct _DataPacket
{
	char filename[FILENAMESIZE];
	int filesize;
	int	nowtranssize;
	char buf[BUFSIZE * 2];
	char packetbuf[BUFSIZE];
};

void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);
_ClientInfo* AddClientInfo(SOCKET sock, SOCKADDR_IN addr, _DataPacket* ptr);
void ReMoveClientInfo(_ClientInfo*);
bool SearchFile(char *filename);

bool SizeRecvn(SOCKET sock, char * buf, int & size);							//��ü ������ ���ú�
bool PacketRecvn(SOCKET sock, char * buf, int size, PROTOCOL & protocol);		//��Ŷ ���ú�
bool GetProtocol(char * buf, PROTOCOL & protocol);

bool Packing(char * buf, int & size);											//��ŷ DONE
bool Packing(char * buf, int nowtranssize, int & size);							//��ŷ NOW_SIZE
bool Packing(char * buf, int nowtranssize, char * packetbuf, int & size);		//��ŷ DATA
bool UnPacking(char * buf, char * filename, int & filesize,int & nowtranssize);	//����ŷ INTRO
bool UnPacking(char * buf, int & nowtranssize);									//����ŷ NOW_SIZE
bool UnPacking(char * buf, int & nowtranssize, char * packetbuf);				//����ŷ DATA
	
_ClientInfo* ClientInfo[MAXCONCLIENT];
int count;

int main(int argc, char* argv[])
{
	int retval;

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
    _DataPacket* packetptr=new _DataPacket;

	ZeroMemory(ClientInfo, sizeof(ClientInfo));
	
	

	while(1){
		// accept()		

		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET){
			err_display("accept()");
			continue;
		}
		
		ZeroMemory(packetptr, sizeof(_DataPacket));

		// ���� �̸��� ũ��ޱ�
		int packetsize;
		PROTOCOL protocol;
		
		SizeRecvn(client_sock, packetptr->buf, packetsize);
		PacketRecvn(client_sock, packetptr->buf, packetsize, protocol);

		if (protocol == INTRO)
		{
			UnPacking(packetptr->buf, packetptr->filename, packetptr->filesize, packetptr->nowtranssize);
		}

		else
		{
			continue;
		}

		printf("-> ���� ���� �̸�: %s\n", packetptr->filename);	
		printf("-> ���� ���� ũ��: %d\n", packetptr->filesize);
		
		
		if(SearchFile(packetptr->filename))	
		{
			FILE * fp = fopen(packetptr->filename, "rb");				//������ ����(r: �б�,b : ���̳ʸ�)
			fseek(fp, 0, SEEK_END);										//������ ������ �̵�
			int fsize=ftell(fp);										//ftell ���ڰ��������� ������ ó���κб����� ũ�⸦ ��� �Լ�
			fclose(fp);													//�� fseek(fp, 0, SEEK_END) �� ptr->filesize = ftell(fp) �� �� ������ ������ ��üũ�⸦ �˱�����

			if(packetptr->filesize==fsize)								//���� �̹� ���� ������ �������ִµ� ����� �����ϴٸ�
			{
				packetptr->nowtranssize=FILEEXIST;
				printf("�����ϴ� ���� ���� �䱸\n");
				printf("FileSender ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", 	inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));	

				Packing(packetptr->buf, packetsize);

				retval = send(client_sock, packetptr->buf, sizeof(packetsize) + packetsize, 0);
				if (retval == SOCKET_ERROR) err_quit("send()");

				closesocket(client_sock);
				continue;
			}

			packetptr->nowtranssize=fsize;								//����� �ҿ����ϴٸ� �׸�ŭ ����
		}
		else
		{
			packetptr->nowtranssize=0;									//�ƿ� ���ٸ� 0 
		}

		_ClientInfo* ClientPtr=AddClientInfo(client_sock, clientaddr, packetptr);		
		
		Packing(packetptr->buf, packetptr->nowtranssize, packetsize);

		retval = send(client_sock, (char*)packetptr->buf, sizeof(packetsize) + packetsize, 0);
		if (retval == SOCKET_ERROR) err_quit("send()");

		// ���� ����
		FILE *fp = fopen(packetptr->filename, "ab");							//������ ����(a: �̾�ޱ�(�����),b : ���̳ʸ�)
		if(fp == NULL){
			perror("���� ����� ����");
			ReMoveClientInfo(ClientPtr);
			continue;
		}

		// ���� ������ �ޱ�
		ClientPtr->transsize=0;

		while(1){

			if (!SizeRecvn(client_sock, packetptr->buf, packetsize))
			{
				break;
			}

			if (!PacketRecvn(client_sock, packetptr->buf, packetsize, protocol))
			{
				break;
			}

			else
			{
				if (protocol == DATA)
				{
					UnPacking(packetptr->buf, packetptr->nowtranssize, packetptr->packetbuf);
				}

				else
				{
					continue;
				}

				fwrite(packetptr->packetbuf, 1, packetptr->nowtranssize, fp);					//���Ͽ��ٰ� ������ ����Ʈ ������ ���� �Լ�
						//�� ������ �մ� ����,�����Ʈ�� ����(1�� ������ 1����Ʈ�� ����),��� ����(10�� ���� ������ ���ڸ�ŭ 10������ 2�ϰ�� 2*10),�� ����
				if(ferror(fp)){
					perror("���� ����� ����");
					break;
				}
				ClientPtr->transsize += packetptr->nowtranssize;
			}
		}																			//�ݺ����� ����� ���
																					//1. ���� 2.�������� 3.��������
		fclose(fp);

		// ���� ��� ���
		if(ClientPtr->transsize == ClientPtr->filesize)
			printf("-> ���� ���� �Ϸ�!\n");
		else
			printf("-> ���� ���� ����!\n");	
		
		ReMoveClientInfo(ClientPtr);
	}

	// closesocket()
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}

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


_ClientInfo* AddClientInfo(SOCKET sock, SOCKADDR_IN addr, _DataPacket* ptr)
{
	ClientInfo[count]=new _ClientInfo;
	ZeroMemory(ClientInfo[count], sizeof(_ClientInfo));
	ClientInfo[count]->sock=sock;
	memcpy(&ClientInfo[count]->addr, &addr, sizeof(addr));
	strcpy(ClientInfo[count]->filename, ptr->filename);
	ClientInfo[count]->filesize=ptr->filesize-ptr->nowtranssize;		//�̹� ���� �κ��� ����� ������ ���� ������ ���� ����� �޴´�
	
	printf("\nFileSender ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", 
		inet_ntoa(ClientInfo[count]->addr.sin_addr), 
		ntohs(ClientInfo[count]->addr.sin_port));
	count++;
	return ClientInfo[count-1];
}

void ReMoveClientInfo(_ClientInfo* ptr)
{
	closesocket(ptr->sock);
	printf("FileSender ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(ptr->addr.sin_addr), ntohs(ptr->addr.sin_port));

	for(int i=0; i<count; i++)
	{
		if (ClientInfo[i] == ptr)
		{				
			delete ptr;
			for(int j=i; j<count-1; j++)
			{
				ClientInfo[j]=ClientInfo[j+1];
			}
			break;
		}
	}

	count--;	
}

bool SearchFile(char *filename)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile = FindFirstFile(filename, &FindFileData);
	if(hFindFile == INVALID_HANDLE_VALUE)
		return false;
	else{
		FindClose(hFindFile);
		return true;
	}
}
// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char * buf, int len, int flags)
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

bool SizeRecvn(SOCKET sock, char * buf, int & size)
{
	int retval = recvn(sock, (char*)&size, sizeof(size), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("SizeRecvn error()");
		return false;
	}

	else if (retval == 0)
	{
		return false;
	}

	return true;
}

bool PacketRecvn(SOCKET sock, char * buf, int size, PROTOCOL & protocol)
{
	int retval = recvn(sock, buf, size, 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("PacketRecvn error()");
		return false;
	}

	else if (retval == 0)
	{
		return false;
	}

	char * ptr = buf;
	memcpy(&protocol, ptr, sizeof(protocol));

	return true;
}

bool Packing(char * buf, int & size)
{
	PROTOCOL protocol = DONE;
	size = sizeof(protocol);

	char * ptr = buf;

	memcpy(ptr, &size, sizeof(size));
	ptr = ptr + sizeof(size);

	memcpy(ptr, &protocol, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	return true;
}

bool Packing(char * buf, int nowtranssize , int & size)
{
	PROTOCOL protocol = NOW_SIZE;
	size = sizeof(protocol) + sizeof(nowtranssize);
	
	char * ptr = buf;

	memcpy(ptr, &size, sizeof(size));
	ptr = ptr + sizeof(size);

	memcpy(ptr, &protocol, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	memcpy(ptr, &nowtranssize, sizeof(nowtranssize));
	ptr = ptr + sizeof(nowtranssize);

	return true;
}

bool Packing(char * buf, int nowtranssize, char * packetbuf, int & size)
{
	PROTOCOL protocol = DATA;
	int  namelen = strlen(packetbuf);
	size = sizeof(protocol) + sizeof(namelen) + namelen;

	char * ptr = buf;

	memcpy(ptr, &size, sizeof(size));
	ptr = ptr + sizeof(size);

	memcpy(ptr, &protocol, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	memcpy(ptr, &nowtranssize, sizeof(nowtranssize));
	ptr = ptr + sizeof(nowtranssize);

	memcpy(ptr, &namelen, sizeof(namelen));
	ptr = ptr + sizeof(namelen);

	memcpy(ptr, &packetbuf, namelen);
	ptr = ptr + namelen;

	return true;
}

bool UnPacking(char * buf, char * filename, int & filesize, int & nowtranssize)
{
	int namelen;

	char * ptr = buf;
	ptr += sizeof(PROTOCOL);

	memcpy(&filesize, ptr, sizeof(filesize));
	ptr += sizeof(filesize);

	memcpy(&nowtranssize, ptr, sizeof(nowtranssize));
	ptr += sizeof(nowtranssize);

	memcpy(&namelen, ptr, sizeof(namelen));
	ptr += sizeof(namelen);

	memcpy(filename, ptr, namelen);
	ptr += namelen;

	return true;
}

bool UnPacking(char * buf, int & nowtranssize)
{
	int namelen;

	char * ptr = buf;
	ptr += sizeof(PROTOCOL);

	memcpy(&nowtranssize, ptr, sizeof(nowtranssize));
	ptr += sizeof(nowtranssize);

	return true;
}

bool UnPacking(char * buf, int & nowtranssize, char * packetbuf)
{
	char * ptr = buf;
	ptr += sizeof(PROTOCOL);

	memcpy(&nowtranssize, ptr, sizeof(nowtranssize));
	ptr += sizeof(nowtranssize);

	memcpy(packetbuf, ptr, BUFSIZE);
	ptr += BUFSIZE;

	return true;
}

