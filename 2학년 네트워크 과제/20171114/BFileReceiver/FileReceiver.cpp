#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 4096
#define FILENAMESIZE 256
#define MAXCONCLIENT 100
#define FILEEXIST -1
#define PLAYERCOUNT 10

enum PROTOCOL{FILE_INFO, TRANS_DENY_INFO, FILE_SIZE_INFO, TRANS_INFO};

/*
��������																������Ʈ
FILE_INFO : _File_info ����												FILE_INFO_RECV
TRANS_DENY_INFO : ���� ����												CONNECT_DONE
FILE_SIZE_INFO : info �۽�												FILE_SIZE_INFO_SEND
TRANS_INFO : info ����, (���� ������ ��ƿ��°� �������� buf)			������ CONNECTING ���ۿϷ� CONNECT_DONE ������ ������� DISCONNECT
*/

struct _File_info
{
	char filename[FILENAMESIZE];
	int  filesize;
	int  info;	
	int  transsize;
	bool isconnect;
};

struct _ClientInfo
{
	SOCKET sock;
	SOCKADDR_IN addr;
	_File_info  file_info;
	char packetbuf[BUFSIZE];
	HANDLE thread;
};

CRITICAL_SECTION cs;
HANDLE hThread[PLAYERCOUNT + 1];
_ClientInfo* ClientInfo[PLAYERCOUNT];
int cl_count = 0;
int th_count = 0;
int fl_count = 0;

void err_quit(char*);
void err_display(char*);
int recvn(SOCKET, char*, int, int);

_ClientInfo* AddClientInfo(SOCKET sock, SOCKADDR_IN addr);
void ReMoveClientInfo(_ClientInfo*);

bool AddThread(_ClientInfo* _ptr);
void RemoveThread(int index);

bool SearchFile(char*);

void PackPacket(char*, PROTOCOL, int, char*, int&);
void PackPacket(char*, PROTOCOL, int, int&);
void GetProtocol(char*, PROTOCOL&);
void UnPackPacket(char*, _File_info&);
void UnPackPacket(char*, char*, int&);

DWORD CALLBACK ProcessClient(LPVOID);							//���������� ����� Ŭ���̾�Ʈ ������ �Լ�
DWORD WINAPI Dummy(LPVOID arg);									//���� ������ �Լ�
DWORD WINAPI WaitingThread(LPVOID arg);							//��� ������ �Լ�

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

	_ClientInfo * ClientPtr = nullptr;
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;	

	hThread[th_count++] = CreateThread(NULL, 0, Dummy, 0, 0, NULL);						//���� ������ �Լ� ����
	HANDLE wating = CreateThread(NULL, 0, WaitingThread, 0, 0, NULL);					//���ÿ� ������ �Լ� ����
	CloseHandle(wating);															

	InitializeCriticalSection(&cs);														//ũ��Ƽ�� ���� ����

	while(1)
	{
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET){
			err_display("accept()");
			continue;
		}	

		ClientPtr = AddClientInfo(client_sock, clientaddr);								//���ο� Ŭ���̾�Ʈ ����

		if (!AddThread(ClientPtr))														//���ο� Ŭ���̾�Ʈ ��� ������ ����
		{
			ReMoveClientInfo(ClientPtr);												//���н� �ֱٿ� ���� Ŭ���̾�Ʈ ����
			continue;
		}
	}

	DeleteCriticalSection(&cs);															//ũ��Ƽ�� ���� ����

	// closesocket()
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}

DWORD WINAPI Dummy(LPVOID arg)
{
	while (1)
	{
		Sleep(0);
	}

	return ERROR;
}


DWORD WINAPI WaitingThread(LPVOID arg)
{
	while (1)
	{
		int index = WaitForMultipleObjects(th_count, hThread, false, INFINITE);

		if (index == 0)																//���� ���� ������ �Լ��� �׾��ٸ�
		{
			hThread[0] = CreateThread(NULL, 0, Dummy, 0, 0, NULL);					//���ο� �Լ��� ���°���
		}																			//�ٽ� WaitForMultipleObjects �� Ȱ���� �ϱ����� ���� ������ �Լ��� ������ش�

		else if (index == -1)
		{
			printf("index ���� -1 �����߻�\n");
			continue;
		}

		else
		{
			printf("%d�� ������ ����\n", index);									//�׿ܿ��� ���� ������ �Լ��� ����Ȱ�

			_ClientInfo * ptr = nullptr;

			for (int i = 0; i < cl_count; i++)
			{
				if (ClientInfo[i]->thread == hThread[index])
				{
					ptr = ClientInfo[i];
					break;
				}
			}

			fl_count--;

			RemoveThread(index);
		}
	}
}

DWORD CALLBACK ProcessClient(LPVOID arg)
{
	EnterCriticalSection(&cs);														//�̹� ������ �����ϰ� �ִµ� �ٸ� Ŭ�� ���ÿ� �����ϴ°� �������� �κ�

	_ClientInfo* ClientPtr = (_ClientInfo*)arg;
	int size;
	PROTOCOL protocol;
	bool fileflag = false;
	bool endflag = false;
	FILE *fp;
	char buf[BUFSIZE];

	while (1)
	{
		int retval = recvn(ClientPtr->sock, (char*)&size, sizeof(size), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("gvalue recv error()");
			endflag = true;
			break;
		}
		else if (retval == 0)
		{
			endflag = true;
			break;
		}

		retval = recvn(ClientPtr->sock, ClientPtr->packetbuf, size, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("gvalue recv error()");
			endflag = true;
			break;

		}
		else if (retval == 0)
		{
			endflag = true;
			break;
		}

		GetProtocol(ClientPtr->packetbuf, protocol);

		switch (protocol)
		{
		case FILE_INFO:
			UnPackPacket(ClientPtr->packetbuf, ClientPtr->file_info);

			printf("-> ���� ���� �̸�: %s\n", ClientPtr->file_info.filename);
			printf("-> ���� ���� ũ��: %d\n", ClientPtr->file_info.filesize);

			if (SearchFile(ClientPtr->file_info.filename))
			{
				fp = fopen(ClientPtr->file_info.filename, "rb");
				fseek(fp, 0, SEEK_END);
				int fsize = ftell(fp);
				fclose(fp);
				if (ClientPtr->file_info.filesize == fsize)
				{
					printf("�����ϴ� ���� ���� �䱸\n");

					PackPacket(ClientPtr->packetbuf, TRANS_DENY_INFO, FILEEXIST, "�����ϰ��� �ϴ� ������ �̹� ������ �����ϴ� �����Դϴ�.", size);

					retval = send(ClientPtr->sock, ClientPtr->packetbuf, size, 0);
					if (retval == SOCKET_ERROR)
					{
						err_display("send()");
					}
					endflag = true;
					break;
				}
				else
				{
					ClientPtr->file_info.info = fsize;
				}

			}
			else
			{
				ClientPtr->file_info.info = 0;
			}


			ClientPtr->file_info.filesize = ClientPtr->file_info.filesize - ClientPtr->file_info.info;

			PackPacket(ClientPtr->packetbuf, FILE_SIZE_INFO, ClientPtr->file_info.info, size);

			retval = send(ClientPtr->sock, ClientPtr->packetbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("send()");
				endflag = true;
				break;
			}

			fileflag = true;
			fp = fopen(ClientPtr->file_info.filename, "ab");

			break;
		case TRANS_INFO:
			UnPackPacket(ClientPtr->packetbuf, buf, ClientPtr->file_info.info);
			fwrite(buf, 1, ClientPtr->file_info.info, fp);
			if (ferror(fp)){
				perror("���� ����� ����");
				endflag = true;
				break;
			}
			ClientPtr->file_info.transsize += ClientPtr->file_info.info;
			break;
		}

		if (endflag)
		{
			break;
		}

	}

	if (fileflag)
	{
		fclose(fp);
	}

	if (ClientPtr->file_info.transsize == ClientPtr->file_info.filesize)
	{
		printf("���ۿϷ�!!\n");
	}
	else
	{
		printf("���۽���!!\n");
	}

	ReMoveClientInfo(ClientPtr);
	LeaveCriticalSection(&cs);																	//������ ������ ũ��Ƽ�� ���� ������
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

bool AddThread(_ClientInfo* _ptr)
{
	EnterCriticalSection(&cs);

	hThread[th_count] = CreateThread(NULL, 0, ProcessClient, _ptr, 0, NULL);
	_ptr->thread = hThread[th_count];

	if (_ptr->thread == NULL)
	{
		LeaveCriticalSection(&cs);
		return false;
	}

	else
	{
		TerminateThread(hThread[0], 0);				//���� ������ �Լ� �͹̳���Ʈ
		th_count++;
		LeaveCriticalSection(&cs);
		return true;
	}
}

void RemoveThread(int index)
{
	EnterCriticalSection(&cs);

	for (int i = index; i < th_count; i++)
	{
		hThread[i] = hThread[i + 1];
	}

	hThread[th_count] = NULL;
	th_count--;

	LeaveCriticalSection(&cs);
}

_ClientInfo* AddClientInfo(SOCKET sock, SOCKADDR_IN addr)
{
	EnterCriticalSection(&cs);

	_ClientInfo* ptr= new _ClientInfo;
	ZeroMemory(ptr, sizeof(_ClientInfo));
	ptr->sock = sock;
	memcpy(&ptr->addr, &addr, sizeof(addr));		
	
	ptr->file_info.isconnect = false;

	printf("\nFileSender ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", 
		inet_ntoa(ptr->addr.sin_addr),ntohs(ptr->addr.sin_port));

	ClientInfo[cl_count++]=ptr;

	LeaveCriticalSection(&cs);
	return ptr;
}

void ReMoveClientInfo(_ClientInfo* ptr)
{
	EnterCriticalSection(&cs);

	closesocket(ptr->sock);
	printf("FileSender ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(ptr->addr.sin_addr), ntohs(ptr->addr.sin_port));

	for(int i=0; i<cl_count; i++)
	{
		if (ClientInfo[i] == ptr)
		{				
			delete ptr;
			for (int j = i; j<cl_count - 1; j++)
			{
				ClientInfo[j]=ClientInfo[j+1];
			}
			break;
		}
	}

	cl_count--;

	LeaveCriticalSection(&cs);
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

void PackPacket(char* _buf, PROTOCOL  _protocol, int _info, char* _str, int& _size) //�������� ����
{
	char* ptr = _buf;
	int strsize = strlen(_str);
	_size = sizeof(_protocol)+sizeof(_info)+sizeof(strsize)+strsize;

	memcpy(ptr, &_size, sizeof(_size));
	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);

	memcpy(ptr, &_info, sizeof(_info));
	ptr = ptr + sizeof(_info);

	memcpy(ptr, &strsize, sizeof(strsize));
	ptr = ptr + sizeof(strsize);

	memcpy(ptr, _str, strsize);	

	_size = _size + sizeof(_size);
}

void PackPacket(char* _buf, PROTOCOL _protocol, int _info, int& _size)//���� ���� ����
{
	char* ptr = _buf;
	_size = sizeof(_protocol)+sizeof(_info);

	memcpy(ptr, &_size, sizeof(_size));
	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);

	memcpy(ptr, &_info, sizeof(_info));
	ptr = ptr + sizeof(_info);

	_size = _size + sizeof(_size);
}

void GetProtocol(char* _buf, PROTOCOL& _protocol)
{
	memcpy(&_protocol, _buf, sizeof(PROTOCOL));
}

void UnPackPacket(char* _buf, _File_info& _finfo)//�����̸� & ���� ũ��
{
	char* ptr = _buf + sizeof(PROTOCOL);
	int size;

	memcpy(&size, ptr, sizeof(size));
	ptr = ptr + sizeof(size);

	memcpy(_finfo.filename, ptr, size);
	ptr = ptr + size;

	memcpy(&_finfo.filesize, ptr, sizeof(_finfo.filesize));
}

void UnPackPacket(char* _buf, char* _transbuf, int& _info) //���� ����
{
	char* ptr = _buf + sizeof(PROTOCOL);	

	memcpy(&_info, ptr, sizeof(_info));
	ptr = ptr + sizeof(_info);

	memcpy(_transbuf, ptr, _info);
}
