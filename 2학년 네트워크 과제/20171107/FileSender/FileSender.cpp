#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 4096
#define FILENAMESIZE 256
#define MAXCONCLIENT 100
#define FILEEXIST -1


enum PROTOCOL { INTRO, NOW_SIZE, DATA, DONE};

/*
1. INTRO : _DataPacket ����ü�� �ۼ����ϴ� ��������
2. NOW_SIZE : nowtranssize �� �ۼ����ϴ� ��������
3. DATA : nowtranssize�� ������ ���ڿ� �� �ۼ����ϴ� ��������
4. DONE : �������ݸ��� �ۼ����ϴ� ��������
*/

struct _DataPacket
{
	char filename[FILENAMESIZE];
	int filesize;
	int nowtranssize;
	char buf[BUFSIZE * 2];
	char packetbuf[BUFSIZE];
};

void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);

bool SizeRecvn(SOCKET sock, char * buf, int & size);							//��ü ������ ���ú�
bool PacketRecvn(SOCKET sock, char * buf, int size, PROTOCOL & protocol);		//��Ŷ ���ú�

bool Packing(char * buf, char * filename, int filesize, int nowtranssize, int & size);	//��ŷ INTRO
bool Packing(char * buf, int nowtranssize, int &size);									//��ŷ NOW_SIZE
bool Packing(char * buf, int nowtranssize, char * packetbuf, int & size);				//��ŷ DATA
bool UnPacking(char * buf, int & nowtranssize);											//����ŷ DONE
bool UnPacking(char * buf, int & nowtranssize, char * packetbuf);						//����ŷ DATA

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

int main(int argc, char* argv[])			//������ �޴� ���ڰ�
		//������ ���� ���ڰ��� ����, ���ڰ���
		//�ܼ�ȯ�濡�� ���ɰ�
		//�巡�� ������ε� ���� ���ڷ� �����°� ����?
{
	int retval;

	

	if(argc < 2){												//������ ���ϸ��� ������� �ʾҴ�
		fprintf(stderr, "Usage: %s <FileName>\n", argv[0]);		//������ �̸��� ����ϴ°�
		return -1;												
	}

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");	
	
	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("connect()");

	_DataPacket* ptr=new _DataPacket;						//����� �ϳ��� ������Ǵ� �������� �������� �����ص� �ȴ�

	ZeroMemory(ptr, sizeof(_DataPacket));

	
	// ���� ����
	FILE *fp = fopen(argv[1], "rb");						//�������ְ� ���̳ʸ� ���·� ������ ����

	if(fp == NULL){
		perror("���� ����� ����");
		return -1;
	}

	strcpy(ptr->filename, argv[1]);							//������ ó���κ����� �����͸� �̵�
	fseek(fp, 0, SEEK_END);									//������ ������ �̵�
	ptr->filesize = ftell(fp);								//ftell ���ڰ��������� ������ ó���κб����� ũ�⸦ ��� �Լ�
															//�� fseek(fp, 0, SEEK_END) �� ptr->filesize = ftell(fp) �� �� ������ ������ ��üũ�⸦ �˱�����
	int size = 0;
	Packing(ptr->buf, ptr->filename, ptr->filesize, ptr->nowtranssize, size);

	retval = send(sock, (char*)ptr->buf, sizeof(size) + size, 0);
	if (retval == SOCKET_ERROR) err_quit("send()");

	PROTOCOL protocol;
	SizeRecvn(sock, ptr->buf, size);
	if (!PacketRecvn(sock, ptr->buf, size, protocol))
	{
		closesocket(sock);
		printf("�����κ��� ������ ���������ϴ�");
		WSACleanup();
		return 0;
	}

	if (protocol == NOW_SIZE)
	{
		UnPacking(ptr->buf, ptr->nowtranssize);
	}

	else if (protocol == DONE)
	{
		printf("�̹� �����ϴ� �����Դϴ�.\n");

		closesocket(sock);
		WSACleanup();
		return 0;
	}

	else
	{
		WSACleanup();
		return -1;
	}

	// ���� ������ ���ۿ� ����� ����
	
	int numtotal = 0;										//�̰��� �Ӵٸ� ������ �޴°��� ������ �ְų� ��� nowtranssize�� 0Ȥ�� ��� ���� ��������

	fseek(fp, ptr->nowtranssize, SEEK_SET);					//nowtranssize �� ������ ��ġ�� �����
	ptr->filesize-=ptr->nowtranssize;						//�������� ��ġ�� �̵��Ѵ�


	// ���� ������ ������

	while(1){
		ptr->nowtranssize = fread(ptr->packetbuf, 1, BUFSIZE, fp);			//�б�

		if(ptr->nowtranssize > 0){										//0����Ʈ���� ������ ���� �����µ� �����Ѵٸ�
			
			Packing(ptr->buf, ptr->nowtranssize, ptr->packetbuf, size);

			retval = send(sock, ptr->buf, sizeof(size) + size, 0);
			if (retval == SOCKET_ERROR) err_quit("send()");

			numtotal += ptr->nowtranssize;
			printf("..");												//�߰��� ���°� �׽�Ʈ�ϱ����� �κ�
			Sleep(100);
		}
		else if(ptr->nowtranssize == 0 && numtotal == ptr->filesize){	//������ ����� 0�̰� ������ ũ��� numtotal �� ���ٸ� ���δ� �۽ŵȰ�
			printf("���� ���� �Ϸ�!: %d ����Ʈ\n", numtotal);
			break;
		}
		else{
			perror("���� ����� ����");
			break;
		}
	}
	fclose(fp);

	// closesocket()
	closesocket(sock);

	delete ptr;

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


// ����� ���� ������ ���� �Լ�
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

bool Packing(char * buf, char * filename, int filesize, int nowtranssize, int & size)
{
	PROTOCOL protocol = INTRO;
	int  namelen = strlen(filename);
	size = sizeof(protocol) + sizeof(namelen) + strlen(filename) + sizeof(filesize) + sizeof(nowtranssize);

	char * ptr = buf;

	memcpy(ptr, &size, sizeof(size));
	ptr = ptr + sizeof(size);

	memcpy(ptr, &protocol, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	memcpy(ptr, &filesize, sizeof(filesize));
	ptr = ptr + sizeof(filesize);

	memcpy(ptr, &nowtranssize, sizeof(nowtranssize));
	ptr = ptr + sizeof(nowtranssize);

	memcpy(ptr, &namelen, sizeof(namelen));
	ptr = ptr + sizeof(namelen);

	memcpy(ptr, filename, namelen);
	ptr = ptr + namelen;
	return true;
}

bool Packing(char * buf, int nowtranssize , int & size)
{
	PROTOCOL protocol = NOW_SIZE;
	size = sizeof(protocol);

	char * ptr = buf;

	memcpy(ptr, &size, sizeof(size));
	ptr = ptr + sizeof(size);

	memcpy(ptr, &protocol, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	memcpy(ptr, &nowtranssize, sizeof(nowtranssize));
	ptr = ptr + sizeof(nowtranssize);

	return true;
}

bool Packing(char * buf, int nowtranssize, char * packetbuf , int & size)
{
	PROTOCOL protocol = DATA;
	size = sizeof(protocol) + sizeof(nowtranssize) + BUFSIZE;

	char * ptr = buf;

	memcpy(ptr, &size, sizeof(size));
	ptr = ptr + sizeof(size);

	memcpy(ptr, &protocol, sizeof(protocol));
	ptr = ptr + sizeof(protocol);

	memcpy(ptr, &nowtranssize, sizeof(nowtranssize));
	ptr = ptr + sizeof(nowtranssize);

	memcpy(ptr, packetbuf, BUFSIZE);
	ptr = ptr + BUFSIZE;

	return true;
}

bool UnPacking(char * buf, int & nowtranssize)
{
	char * ptr = buf;
	ptr += sizeof(PROTOCOL);

	memcpy(&nowtranssize, ptr, sizeof(nowtranssize));
	ptr += sizeof(nowtranssize);

	return true;
}

bool UnPacking(char * buf, int & nowtranssize, char * packetbuf)
{
	int namelen;

	char * ptr = buf;
	ptr += sizeof(PROTOCOL);

	memcpy(&nowtranssize, ptr, sizeof(nowtranssize));
	ptr += sizeof(nowtranssize);

	memcpy(&namelen, ptr, sizeof(namelen));
	ptr += sizeof(namelen);

	memcpy(packetbuf, ptr, namelen);
	ptr += namelen;

	return true;
}