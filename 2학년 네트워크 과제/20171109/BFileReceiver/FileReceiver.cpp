#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 4096
#define FILENAMESIZE 256
#define MAXCONCLIENT 100
#define FILEEXIST -1

enum PROTOCOL{ INTRO, NOW_SIZE, DATA, DONE};

/*
1. INTRO : _DataPacket 구조체를 송수신하는 프로토콜
2. NOW_SIZE : nowtranssize 를 송수신하는 프로토콜
3. DATA : nowtranssize과 임의의 문자열 을 송수신하는 프로토콜
4. DONE : 프로토콜만을 송수신하는 프로토콜
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

bool SizeRecvn(SOCKET sock, char * buf, int & size);							//전체 사이즈 리시브
bool PacketRecvn(SOCKET sock, char * buf, int size, PROTOCOL & protocol);		//패킷 리시브
bool GetProtocol(char * buf, PROTOCOL & protocol);

bool Packing(char * buf, int & size);											//패킹 DONE
bool Packing(char * buf, int nowtranssize, int & size);							//패킹 NOW_SIZE
bool Packing(char * buf, int nowtranssize, char * packetbuf, int & size);		//패킹 DATA
bool UnPacking(char * buf, char * filename, int & filesize,int & nowtranssize);	//언패킹 INTRO
bool UnPacking(char * buf, int & nowtranssize);									//언패킹 NOW_SIZE
bool UnPacking(char * buf, int & nowtranssize, char * packetbuf);				//언패킹 DATA
	
_ClientInfo* ClientInfo[MAXCONCLIENT];
int count;

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
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

	// 데이터 통신에 사용할 변수

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

		// 파일 이름과 크기받기
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

		printf("-> 받을 파일 이름: %s\n", packetptr->filename);	
		printf("-> 받을 파일 크기: %d\n", packetptr->filesize);
		
		
		if(SearchFile(packetptr->filename))	
		{
			FILE * fp = fopen(packetptr->filename, "rb");				//파일을 연다(r: 읽기,b : 바이너리)
			fseek(fp, 0, SEEK_END);										//파일의 끝으로 이동
			int fsize=ftell(fp);										//ftell 인자값에서부터 파일의 처음부분까지의 크기를 재는 함수
			fclose(fp);													//즉 fseek(fp, 0, SEEK_END) 와 ptr->filesize = ftell(fp) 를 한 이유는 파일의 전체크기를 알기위함

			if(packetptr->filesize==fsize)								//만약 이미 같은 파일을 가지고있는데 사이즈가 완전하다면
			{
				packetptr->nowtranssize=FILEEXIST;
				printf("존재하는 파일 전송 요구\n");
				printf("FileSender 종료: IP 주소=%s, 포트 번호=%d\n", 	inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));	

				Packing(packetptr->buf, packetsize);

				retval = send(client_sock, packetptr->buf, sizeof(packetsize) + packetsize, 0);
				if (retval == SOCKET_ERROR) err_quit("send()");

				closesocket(client_sock);
				continue;
			}

			packetptr->nowtranssize=fsize;								//사이즈가 불완전하다면 그만큼 저장
		}
		else
		{
			packetptr->nowtranssize=0;									//아에 없다면 0 
		}

		_ClientInfo* ClientPtr=AddClientInfo(client_sock, clientaddr, packetptr);		
		
		Packing(packetptr->buf, packetptr->nowtranssize, packetsize);

		retval = send(client_sock, (char*)packetptr->buf, sizeof(packetsize) + packetsize, 0);
		if (retval == SOCKET_ERROR) err_quit("send()");

		// 파일 열기
		FILE *fp = fopen(packetptr->filename, "ab");							//파일을 연다(a: 이어받기(어펜드),b : 바이너리)
		if(fp == NULL){
			perror("파일 입출력 오류");
			ReMoveClientInfo(ClientPtr);
			continue;
		}

		// 파일 데이터 받기
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

				fwrite(packetptr->packetbuf, 1, packetptr->nowtranssize, fp);					//파일에다가 내용을 바이트 단위로 쓰는 함수
						//쓸 내용이 잇는 버퍼,몇바이트씩 쓸지(1을 썼으니 1바이트씩 쓴다),몇번 쓸지(10을 쓰면 왼쪽의 인자만큼 10번쓴다 2일경우 2*10),쓸 파일
				if(ferror(fp)){
					perror("파일 입출력 오류");
					break;
				}
				ClientPtr->transsize += packetptr->nowtranssize;
			}
		}																			//반복문이 종료될 경우
																					//1. 오류 2.정상종료 3.연결종료
		fclose(fp);

		// 전송 결과 출력
		if(ClientPtr->transsize == ClientPtr->filesize)
			printf("-> 파일 전송 완료!\n");
		else
			printf("-> 파일 전송 실패!\n");	
		
		ReMoveClientInfo(ClientPtr);
	}

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
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


_ClientInfo* AddClientInfo(SOCKET sock, SOCKADDR_IN addr, _DataPacket* ptr)
{
	ClientInfo[count]=new _ClientInfo;
	ZeroMemory(ClientInfo[count], sizeof(_ClientInfo));
	ClientInfo[count]->sock=sock;
	memcpy(&ClientInfo[count]->addr, &addr, sizeof(addr));
	strcpy(ClientInfo[count]->filename, ptr->filename);
	ClientInfo[count]->filesize=ptr->filesize-ptr->nowtranssize;		//이미 받은 부분의 사이즈를 뺴내어 내가 앞으로 받을 사이즈를 받는다
	
	printf("\nFileSender 접속: IP 주소=%s, 포트 번호=%d\n", 
		inet_ntoa(ClientInfo[count]->addr.sin_addr), 
		ntohs(ClientInfo[count]->addr.sin_port));
	count++;
	return ClientInfo[count-1];
}

void ReMoveClientInfo(_ClientInfo* ptr)
{
	closesocket(ptr->sock);
	printf("FileSender 종료: IP 주소=%s, 포트 번호=%d\n",
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
// 사용자 정의 데이터 수신 함수
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

