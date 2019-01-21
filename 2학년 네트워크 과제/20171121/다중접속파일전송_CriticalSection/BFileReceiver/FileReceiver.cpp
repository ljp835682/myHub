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

//어떤 클라이언트가 끝낫을떄 그 클라의 뒤에서 기다리고있던 클라이언트가 있나 확인하는 함수
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

	// 윈속 초기화
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

	// 데이터 통신에 사용할 변수

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
	// 윈속 종료
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
		case FILE_INIT_TRANS_INFO:														//맨 처음에 클라가 보내는 패킷
			UnPackPacket(ClientPtr->packetbuf, ClientPtr->file_info);

			printf("-> 받을 파일 이름: %s\n", ClientPtr->file_info.filename);
			printf("-> 받을 파일 크기: %d\n", ClientPtr->file_info.filesize);
			
			EnterCriticalSection(&cs);

			if (SearchFile(ClientPtr->file_info.filename))								//같은 파일을 가지고 있나
			{
				fp = fopen(ClientPtr->file_info.filename, "rb");
				fseek(fp, 0, SEEK_END);
				int fsize = ftell(fp);
				fclose(fp);
					
				if (ClientPtr->file_info.filesize == fsize)								//완료된 사이즈 일경우
				{
					LeaveCriticalSection(&cs);

					ClientPtr->state = FILE_TRANS_DENY_SEND;

					printf("존재하는 파일 전송 요구\n");

					PackPacket(ClientPtr->packetbuf, TRANS_DENY_INFO, FILE_EXIST, "전송하고자 하는 파일은 이미 서버에 존재하는 파일입니다.", size);

					retval = send(ClientPtr->sock, ClientPtr->packetbuf, size, 0);
					if (retval == SOCKET_ERROR)
					{
						err_display("send()");
					}
					
					return 0;
					
				}
				/*
				
				파일이 있는데 크기가 다르다면 전송중이 아닌데 끊고 나갔거나
				누군가가 전송중인 경우

				*/


				_ClientInfo* ptr = SearchClientInfo(ClientPtr->file_info.filename);
				if (ptr != NULL && ptr != ClientPtr)							//이미 보내고있는 클라가 있는경우
				{
					LeaveCriticalSection(&cs);									//크리티컬 세션 반납

					ClientPtr->state = FILE_TRANS_WAIT;	
					PackPacket(ClientPtr->packetbuf,
								FILE_TRANS_INFO,
							    TRANS_WAIT,
							   "다른 Sender가 전송중인 파일입니다. 잠시 결과를 기다리십시오",		//패킷 만들어서 보냄 
							   size);

					retval = send(ClientPtr->sock, ClientPtr->packetbuf, size, 0);
					if (retval == SOCKET_ERROR)
					{
						err_display("send()");
					}

					return 0;													//스레드 종료
				}				

				ClientPtr->file_info.info = fsize;								//이어서 보내기
				

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
				perror("파일 입출력 오류");
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
		printf("파일수신완료!!\n");
		ClientPtr->state = FILE_TRANS_END;				//상태변경
	}
	else
	{
		printf("파일수신실패!!\n");						//만약 완료되지 않고 끊으면 FILE_DATA_RECV 상태에서 끝남
	}
	
	ClientPtr->file_info.trans = false;					//파일 전송 끝남
														//지금 사용하지않음
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

		printf("쓰레드 종료\n");

		EnterCriticalSection(&cs);
		
		_ClientInfo* ptr = SearchClientInfo(hThread[index]);				//지금 끝난 클라이언트를 찾아낸다
	
		switch (ptr->state)
		{
		case FILE_TRANS_WAIT:												//파일 전송 기다리기 할거 없음
			break;
		case FILE_INIT_TRANS_INFO_RECV:										//초기화 실패
			ReMoveClientInfo(ptr);
			break;
		case FILE_TRANS_POINT_INFO_SEND:									//파일 전송 실패
			ReMoveClientInfo(ptr);
			break;
		case FILE_DATA_RECV:												//파일 전송 거부 종료
			WakeUpWaitClient(ptr);											//한개의 클라이언트만 열어야함
			ReMoveClientInfo(ptr);											
			break;
		
		case FILE_TRANS_DENY_SEND:											//이미 전송된 파일을 전송했을경우
			printf("파일수신거부!!\n");										//FILE_TRANS_END 가 발생하면 뒤의 클라이언트들은 FILE_TRANS_DENY_SEND 상태가 될것
			WakeUpWaitClient(ptr);											//실패했기에 전송을 잇기위해서 뒤의 클라이언트를 꺠움
			ReMoveClientInfo(ptr);
			break;
		case FILE_TRANS_END:												//파일 전송 성공 종료
			WakeUpWaitClient(ptr);											//성공했기에 뒤의 클라들을 연쇄적으로 종료시키기위해 클라이언트들을 꺠움
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
		if (ClientInfo[i]->state == FILE_TRANS_WAIT)														//전송을 기다리고 있는 클라이언트일경우
		{	
			if (!strcmp(_ptr->file_info.filename, ClientInfo[i]->file_info.filename))						//지금 끝난 클라이언드와 동일한 파일명을 가진 클라를 찾아냄
			{
				
				PackPacket(ClientInfo[i]->packetbuf, FILE_TRANS_INFO, TRANS_RESTART, "다시 전송 시도", size);		//다시 전송 시도
				retval = send(ClientInfo[i]->sock, ClientInfo[i]->packetbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("send()");
				}

				AddThread(ProcessClient, ClientInfo[i]);			
				break;																								//맨앞에 있는 클라이언트만 깨운다
			}
		}
	}

	LeaveCriticalSection(&cs);
}
// 소켓 함수 오류 출력 후 종료
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


_ClientInfo* AddClientInfo(SOCKET sock, SOCKADDR_IN addr)
{
	printf("\nFileSender 접속: IP 주소=%s, 포트 번호=%d\n",
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
	printf("FileSender 종료: IP 주소=%s, 포트 번호=%d\n",
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
// 사용자 정의 데이터 수신 함수
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
void PackPacket(char* _buf, PROTOCOL  _protocol, int _info, char* _str, int& _size) //파일존재 정보
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


void PackPacket(char* _buf, PROTOCOL _protocol, int _info, int& _size)//파일 길이 정보
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

void UnPackPacket(char* _buf, _File_info& _finfo)//파일이름 & 파일 크기
{
	char* ptr = _buf + sizeof(PROTOCOL);
	int size;

	memcpy(&size, ptr, sizeof(size));
	ptr = ptr + sizeof(size);

	memcpy(_finfo.filename, ptr, size);
	ptr = ptr + size;

	memcpy(&_finfo.filesize, ptr, sizeof(_finfo.filesize));
}

void UnPackPacket(char* _buf, char* _transbuf, int& _info) //파일 전송
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

