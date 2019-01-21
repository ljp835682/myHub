#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 4096
#define FILENAMESIZE 256
#define MAXCONCLIENT 100
#define FILE_EXIST -1
#define TRANS_WAIT 100
#define TRANS_RESTART 200

enum PROTOCOL{FILE_TRANS_INFO,FILE_INIT_TRANS_INFO, TRANS_DENY_INFO, FILE_TRANS_POINT_INFO, FILE_DATA_TRANS};
enum STATE{ FILE_INIT_TRANS_INFO_RECV, FILE_TRANS_POINT_INFO_SEND, FILE_TRANS_DENY_SEND, FILE_DATA_RECV, FILE_TRANS_WAIT, FILE_TRANS_END };

struct _File_info
{
	char filename[FILENAMESIZE];
	int  filesize;
	int  info;
	int  transsize;
	bool trans;
};

struct _ClientInfo
{
	SOCKET sock;
	SOCKADDR_IN addr;
	_File_info  file_info;
	STATE state;
	HANDLE hthread;
	char packetbuf[BUFSIZE];
};



void err_quit(char*);
void err_display(char*);

int recvn(SOCKET, char*, int, int);

_ClientInfo* AddClientInfo(SOCKET sock, SOCKADDR_IN addr);
void ReMoveClientInfo(_ClientInfo*);

DWORD CALLBACK ProcessClient(LPVOID);
DWORD CALLBACK WaitClient(LPVOID); 
DWORD CALLBACK Dummy(LPVOID);

_ClientInfo* SearchClientInfo(char*);
_ClientInfo* SearchClientInfo(HANDLE);
_ClientInfo* SearchClientInfo(SOCKET);

//� Ŭ���̾�Ʈ�� �������� �� Ŭ���� �ڿ��� ��ٸ����ִ� Ŭ���̾�Ʈ�� �ֳ� Ȯ���ϴ� �Լ�
void WakeUpWaitClient(_ClientInfo*);

void AddThread(LPTHREAD_START_ROUTINE, LPVOID);
void RemoveThread(int);

bool SearchFile(char*);

void PackPacket(char*, PROTOCOL, int, char*, int&);
void PackPacket(char*, PROTOCOL, int, int&);

void GetProtocol(char*, PROTOCOL&);

void UnPackPacket(char*, _File_info&);
void UnPackPacket(char*, char*, int&);

_ClientInfo* ClientInfo[MAXCONCLIENT];
int count=0;

HANDLE hThread[MAXCONCLIENT];
int threadcount = 0;

CRITICAL_SECTION	cs;

int main(int argc, char* argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return -1;

	InitializeCriticalSection(&cs);
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
	_ClientInfo* ClientPtr;


	hThread[threadcount++] = CreateThread(nullptr, 0, Dummy, nullptr, 0, nullptr);

	CreateThread(nullptr, 0, WaitClient, nullptr, 0, nullptr);

	while(1)
	{
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET)
		{
			err_display("accept()");
			continue;
		}	

		ClientPtr = AddClientInfo(client_sock, clientaddr);
		AddThread(ProcessClient, ClientPtr);
		TerminateThread(hThread[0], -1);		
		
	}//outer while end

	// closesocket()
	closesocket(listen_sock);

	DeleteCriticalSection(&cs);
	// ���� ����
	WSACleanup();
	return 0;
}

DWORD CALLBACK ProcessClient(LPVOID _ptr)
{
	_ClientInfo* ClientPtr = (_ClientInfo*)_ptr;

	int size;
	PROTOCOL protocol;
	char buf[BUFSIZE];
	FILE *fp;
	int retval;

	bool fileflag = false;
	bool endflag = false;

	while (1)
	{

		retval = recvn(ClientPtr->sock, (char*)&size, sizeof(size), 0);
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
		case FILE_INIT_TRANS_INFO:														//�� ó���� Ŭ�� ������ ��Ŷ
			UnPackPacket(ClientPtr->packetbuf, ClientPtr->file_info);

			printf("-> ���� ���� �̸�: %s\n", ClientPtr->file_info.filename);
			printf("-> ���� ���� ũ��: %d\n", ClientPtr->file_info.filesize);
			
			EnterCriticalSection(&cs);

			if (SearchFile(ClientPtr->file_info.filename))								//���� ������ ������ �ֳ�
			{
				fp = fopen(ClientPtr->file_info.filename, "rb");
				fseek(fp, 0, SEEK_END);
				int fsize = ftell(fp);
				fclose(fp);
					
				if (ClientPtr->file_info.filesize == fsize)								//�Ϸ�� ������ �ϰ��
				{
					LeaveCriticalSection(&cs);

					ClientPtr->state = FILE_TRANS_DENY_SEND;

					printf("�����ϴ� ���� ���� �䱸\n");

					PackPacket(ClientPtr->packetbuf, TRANS_DENY_INFO, FILE_EXIST, "�����ϰ��� �ϴ� ������ �̹� ������ �����ϴ� �����Դϴ�.", size);

					retval = send(ClientPtr->sock, ClientPtr->packetbuf, size, 0);
					if (retval == SOCKET_ERROR)
					{
						err_display("send()");
					}
					
					return 0;
					
				}
				/*
				
				������ �ִµ� ũ�Ⱑ �ٸ��ٸ� �������� �ƴѵ� ���� �����ų�
				�������� �������� ���

				*/


				_ClientInfo* ptr = SearchClientInfo(ClientPtr->file_info.filename);
				if (ptr != NULL && ptr != ClientPtr)							//�̹� �������ִ� Ŭ�� �ִ°��
				{
					LeaveCriticalSection(&cs);									//ũ��Ƽ�� ���� �ݳ�

					ClientPtr->state = FILE_TRANS_WAIT;	
					PackPacket(ClientPtr->packetbuf,
								FILE_TRANS_INFO,
							    TRANS_WAIT,
							   "�ٸ� Sender�� �������� �����Դϴ�. ��� ����� ��ٸ��ʽÿ�",		//��Ŷ ���� ���� 
							   size);

					retval = send(ClientPtr->sock, ClientPtr->packetbuf, size, 0);
					if (retval == SOCKET_ERROR)
					{
						err_display("send()");
					}

					return 0;													//������ ����
				}				

				ClientPtr->file_info.info = fsize;								//�̾ ������
				

			}

			else
			{
				ClientPtr->file_info.info = 0;
			}

			ClientPtr->state = FILE_TRANS_POINT_INFO_SEND;

			ClientPtr->file_info.filesize = ClientPtr->file_info.filesize - ClientPtr->file_info.info;

			PackPacket(ClientPtr->packetbuf, FILE_TRANS_POINT_INFO, ClientPtr->file_info.info, size);

			retval = send(ClientPtr->sock, ClientPtr->packetbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("send()");
				endflag = true;
				break;
			}

			ClientPtr->state = FILE_DATA_RECV;
			fileflag = true;
			ClientPtr->file_info.trans = true;
			fp = fopen(ClientPtr->file_info.filename, "ab");	
			
			LeaveCriticalSection(&cs);
			break;

		case FILE_DATA_TRANS:
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

	}//inner while end

	if (fileflag)
	{
		fclose(fp);
	}

	if (ClientPtr->file_info.transsize == ClientPtr->file_info.filesize)
	{
		printf("���ϼ��ſϷ�!!\n");
		ClientPtr->state = FILE_TRANS_END;				//���º���
	}
	else
	{
		printf("���ϼ��Ž���!!\n");						//���� �Ϸ���� �ʰ� ������ FILE_DATA_RECV ���¿��� ����
	}
	
	ClientPtr->file_info.trans = false;					//���� ���� ����
														//���� �����������
	return 0;	
}

DWORD CALLBACK WaitClient(LPVOID)
{
	int index;
	int retval;

	while (1)
	{
		retval = WaitForMultipleObjects(threadcount, hThread, false, INFINITE);	

		index = retval - WAIT_OBJECT_0;

		DWORD threadid;

		if (index == 0)
		{
			hThread[0] = CreateThread(nullptr, 0, Dummy, nullptr, 0, &threadid);		
			continue;
		}

		printf("������ ����\n");

		EnterCriticalSection(&cs);
		
		_ClientInfo* ptr = SearchClientInfo(hThread[index]);				//���� ���� Ŭ���̾�Ʈ�� ã�Ƴ���
	
		switch (ptr->state)
		{
		case FILE_TRANS_WAIT:												//���� ���� ��ٸ��� �Ұ� ����
			break;
		case FILE_INIT_TRANS_INFO_RECV:										//�ʱ�ȭ ����
			ReMoveClientInfo(ptr);
			break;
		case FILE_TRANS_POINT_INFO_SEND:									//���� ���� ����
			ReMoveClientInfo(ptr);
			break;
		case FILE_DATA_RECV:												//���� ���� �ź� ����
			WakeUpWaitClient(ptr);											//�Ѱ��� Ŭ���̾�Ʈ�� �������
			ReMoveClientInfo(ptr);											
			break;
		
		case FILE_TRANS_DENY_SEND:											//�̹� ���۵� ������ �����������
			printf("���ϼ��Űź�!!\n");										//FILE_TRANS_END �� �߻��ϸ� ���� Ŭ���̾�Ʈ���� FILE_TRANS_DENY_SEND ���°� �ɰ�
			WakeUpWaitClient(ptr);											//�����߱⿡ ������ �ձ����ؼ� ���� Ŭ���̾�Ʈ�� �ƿ�
			ReMoveClientInfo(ptr);
			break;
		case FILE_TRANS_END:												//���� ���� ���� ����
			WakeUpWaitClient(ptr);											//�����߱⿡ ���� Ŭ����� ���������� �����Ű������ Ŭ���̾�Ʈ���� �ƿ�
			ReMoveClientInfo(ptr);
			break;
		
		}
		RemoveThread(index);

		LeaveCriticalSection(&cs);
	
	}

	return 0;
}

void WakeUpWaitClient(_ClientInfo* _ptr)								
{
	int i = 0;
	int size;
	int retval;
	EnterCriticalSection(&cs);
	for (i = 0; i<count; i++)
	{
		if (ClientInfo[i]->state == FILE_TRANS_WAIT)														//������ ��ٸ��� �ִ� Ŭ���̾�Ʈ�ϰ��
		{	
			if (!strcmp(_ptr->file_info.filename, ClientInfo[i]->file_info.filename))						//���� ���� Ŭ���̾��� ������ ���ϸ��� ���� Ŭ�� ã�Ƴ�
			{
				
				PackPacket(ClientInfo[i]->packetbuf, FILE_TRANS_INFO, TRANS_RESTART, "�ٽ� ���� �õ�", size);		//�ٽ� ���� �õ�
				retval = send(ClientInfo[i]->sock, ClientInfo[i]->packetbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("send()");
				}

				AddThread(ProcessClient, ClientInfo[i]);			
				break;																								//�Ǿտ� �ִ� Ŭ���̾�Ʈ�� �����
			}
		}
	}

	LeaveCriticalSection(&cs);
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


_ClientInfo* AddClientInfo(SOCKET sock, SOCKADDR_IN addr)
{
	printf("\nFileSender ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	EnterCriticalSection(&cs);
	_ClientInfo* ptr= new _ClientInfo;
	ZeroMemory(ptr, sizeof(_ClientInfo));
	ptr->sock = sock;
	memcpy(&ptr->addr, &addr, sizeof(addr));
	ptr->file_info.trans = false;
	ptr->state = FILE_INIT_TRANS_INFO_RECV;
	ClientInfo[count++]=ptr;
	LeaveCriticalSection(&cs);
	return ptr;
}

void ReMoveClientInfo(_ClientInfo* ptr)
{
	printf("FileSender ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(ptr->addr.sin_addr), ntohs(ptr->addr.sin_port));
	
	EnterCriticalSection(&cs);
	
	closesocket(ptr->sock);
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
	LeaveCriticalSection(&cs);
}

bool SearchFile(char *filename)
{
	EnterCriticalSection(&cs);
	bool returnflag;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile = FindFirstFile(filename, &FindFileData);
	if (hFindFile == INVALID_HANDLE_VALUE)
		returnflag = false;	    
	else{
		FindClose(hFindFile);
		returnflag = true;		
	}	
	LeaveCriticalSection(&cs);
	return returnflag;
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

void AddThread(LPTHREAD_START_ROUTINE _process, LPVOID _ptr)
{
	EnterCriticalSection(&cs);
	_ClientInfo* ptr = (_ClientInfo*)_ptr;
	ptr->hthread = CreateThread(NULL, 0, _process, ptr, 0, nullptr);
	hThread[threadcount++] = ptr->hthread;
	LeaveCriticalSection(&cs);
}

void RemoveThread(int _index)
{
	EnterCriticalSection(&cs);
	int i;
	for (i = _index; i < threadcount - 1; i++)
	{
		hThread[i] = hThread[i + 1];
	}

	hThread[i] = nullptr;
	threadcount--;
	LeaveCriticalSection(&cs);
}

_ClientInfo* SearchClientInfo(char* _filename)
{
	EnterCriticalSection(&cs);
	for (int i = 0; i<count; i++)
	{
		
		if (ClientInfo[i]->file_info.trans && !strcmp(ClientInfo[i]->file_info.filename, _filename))
		{
			LeaveCriticalSection(&cs);
			return ClientInfo[i];
		}
	}
	LeaveCriticalSection(&cs);
	return NULL;
}

_ClientInfo* SearchClientInfo(HANDLE _hThread)
{
	EnterCriticalSection(&cs);
	for (int i = 0; i<count; i++)
	{
		HANDLE h = ClientInfo[i]->hthread;
		if (ClientInfo[i]->hthread == _hThread)
		{
			LeaveCriticalSection(&cs);
			return ClientInfo[i];
		}
	}

	LeaveCriticalSection(&cs);
	return NULL;

}

_ClientInfo* SearchClientInfo(SOCKET _sock)
{
	EnterCriticalSection(&cs);
	for (int i = 0; i<count; i++)
	{
		if (ClientInfo[i]->sock == _sock)
		{
			LeaveCriticalSection(&cs);
			return ClientInfo[i];
		}
	}
	LeaveCriticalSection(&cs);
	return NULL;
}

DWORD CALLBACK Dummy(LPVOID)
{
	while (1)
	{
		Sleep(0);
	}

	return 0;
}

