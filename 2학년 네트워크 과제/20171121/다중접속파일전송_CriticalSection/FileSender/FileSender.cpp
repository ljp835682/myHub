#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 4096
#define MAX_FILE_READ_SIZE 2048
#define FILENAMESIZE 256
#define MAXCONCLIENT 100
#define FILE_EXIST -1
#define TRANS_WAIT 100
#define TRANS_RESTART 200

enum PROTOCOL{ FILE_TRANS_INFO, FILE_INIT_TRANS_INFO, TRANS_DENY_INFO, FILE_TRANS_POINT_INFO, FILE_DATA_TRANS };

struct _FileInfo
{
	char filename[FILENAMESIZE];
	int filesize;
	int info;
	int  nowtranssize;
	char buf[BUFSIZE];
}Fileinfo;

void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);

void PackPacket(char*, PROTOCOL, _FileInfo, int&);
void PackPacket(char*, PROTOCOL, char*, int, int&);

void GetProtocol(char*, PROTOCOL&);

void UnPackPacket(char*, int&, char*);
void UnPackPacket(char*, int&);

int main(int argc, char* argv[])
{
	int retval;
	char buf[BUFSIZE];

	if(argc < 2){
		fprintf(stderr, "Usage: %s <FileName>\n", argv[0]);
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
		
		
	// ���� ����
	FILE *fp = fopen(argv[1], "rb");

	if(fp == NULL){
		perror("���� ����� ����");
		return -1;
	}

	ZeroMemory(&Fileinfo, sizeof(Fileinfo));

	strcpy(Fileinfo.filename, argv[1]);

	bool breakflag = false;
	
	
	while (1)
	{
		fseek(fp, 0, SEEK_END);
		Fileinfo.filesize = ftell(fp);

		int size;
		PackPacket(buf, FILE_INIT_TRANS_INFO, Fileinfo, size);

		retval = send(sock, buf, size, 0);
		if (retval == SOCKET_ERROR) err_quit("send()");				//������
		
			

		while (1)
		{
			retval = recvn(sock, (char*)&size, sizeof(size), 0);	//��� �ޱ�
			if (retval == SOCKET_ERROR) err_quit("recen()");
			else if (retval == 0)
			{
				closesocket(sock);
				printf("�����κ��� ������ ���������ϴ�");
				WSACleanup();
				return 0;
			}

			retval = recvn(sock, buf, size, 0);
			if (retval == SOCKET_ERROR) err_quit("recen()");
			else if (retval == 0)
			{
				closesocket(sock);
				printf("�����κ��� ������ ���������ϴ�");
				WSACleanup();
				return 0;
			}

			PROTOCOL protocol;
			GetProtocol(buf, protocol);

			char msgbuf[BUFSIZE] = { 0 };

			switch (protocol)										//���ù��� ���´� 3���߿� �Ѱ��ϰ�
			{
			case FILE_TRANS_INFO:									//���� ����
				UnPackPacket(buf, Fileinfo.info, msgbuf);
				switch (Fileinfo.info)
				{
				case TRANS_WAIT:
					printf("%s \n", msgbuf);
					break;
				case TRANS_RESTART:									//���� ��ٷ����� ���
					printf("%s \n", msgbuf);
					breakflag = true;								//�ٽ� ���ù����� ������ϱ�����
																	//�ݺ����� �� �����ϱ����� ������
				}
				break;
			case TRANS_DENY_INFO:									//�ź�
				UnPackPacket(buf, Fileinfo.info, msgbuf);
				if (Fileinfo.info == FILE_EXIST)
				{
					printf("%s \n", msgbuf);
					closesocket(sock);
					WSACleanup();
					return 0;
				}
				break;
			case FILE_TRANS_POINT_INFO:								//��ġ�� ����
				UnPackPacket(buf, Fileinfo.info);
				breakflag = true;
				break;


			}

			if (breakflag)
			{
				break;
			}

		}

		if (Fileinfo.info == TRANS_RESTART)
		{
			continue;
		}

		break;
	}
	

	
	
	// ���� ������ ���ۿ� ����� ����
	
	int numtotal = 0;

	fseek(fp, Fileinfo.info, SEEK_SET);
	Fileinfo.filesize -= Fileinfo.info;


	// ���� ������ ������
	

	while(1){
		Fileinfo.nowtranssize = fread(Fileinfo.buf, 1, MAX_FILE_READ_SIZE, fp);
		if (Fileinfo.nowtranssize > 0)
		{
			int size;
			PackPacket(buf, FILE_DATA_TRANS, Fileinfo.buf, Fileinfo.nowtranssize, size);
			retval = send(sock, buf, size, 0);
			if(retval == SOCKET_ERROR){
				err_display("send()");
				break;
			}
			numtotal += Fileinfo.nowtranssize;
			printf("..");
			Sleep(300);
		}
		else if (Fileinfo.nowtranssize == 0 && numtotal == Fileinfo.filesize){
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

void PackPacket(char* _buf, PROTOCOL _protocol, _FileInfo _finfo, int& _size)
{
	char* ptr = _buf;
	int strsize= strlen(_finfo.filename);
	_size = sizeof(_protocol)+sizeof(strsize)+strsize + sizeof(_finfo.filesize);

	memcpy(ptr, &_size, sizeof(_size));
	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);

	memcpy(ptr, &strsize, sizeof(strsize));
	ptr = ptr + sizeof(strsize);

	memcpy(ptr, _finfo.filename, strsize);
	ptr = ptr + strsize;

	memcpy(ptr, &_finfo.filesize, sizeof(_finfo.filesize));

	_size = _size + sizeof(_size);
}

void PackPacket(char* _buf, PROTOCOL _protocol, char* _filedata, int _filedata_size, int& _size)
{
	char* ptr = _buf;	
	_size = sizeof(_protocol)+sizeof(_filedata_size)+_filedata_size;

	memcpy(ptr, &_size, sizeof(_size));
	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);

	memcpy(ptr, &_filedata_size, sizeof(_filedata_size));
	ptr = ptr + sizeof(_filedata_size);

	memcpy(ptr, _filedata, _filedata_size);	

	_size = _size + sizeof(_size);

}

void GetProtocol(char* _buf, PROTOCOL& _protocol)
{
	memcpy(&_protocol, _buf, sizeof(PROTOCOL));
}

void UnPackPacket(char* _buf, int& _info, char* _msgbuf)
{
	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&_info, ptr, sizeof(_info));
	ptr = ptr + sizeof(_info);

	int size;
	memcpy(&size, ptr, sizeof(size));
	ptr = ptr + sizeof(size);

	memcpy(_msgbuf, ptr, size);
}

void UnPackPacket(char* _buf, int& _info)
{
	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&_info, ptr, sizeof(_info));	
}