#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 4096
#define FILENAMESIZE 256
#define MAXCONCLIENT 100
#define FILEEXIST -1


enum PROTOCOL { INTRO, NOW_SIZE, DATA, DONE};

/*
1. INTRO : _DataPacket 구조체를 송수신하는 프로토콜
2. NOW_SIZE : nowtranssize 를 송수신하는 프로토콜
3. DATA : nowtranssize과 임의의 문자열 을 송수신하는 프로토콜
4. DONE : 프로토콜만을 송수신하는 프로토콜
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

bool SizeRecvn(SOCKET sock, char * buf, int & size);							//전체 사이즈 리시브
bool PacketRecvn(SOCKET sock, char * buf, int size, PROTOCOL & protocol);		//패킷 리시브

bool Packing(char * buf, char * filename, int filesize, int nowtranssize, int & size);	//패킹 INTRO
bool Packing(char * buf, int nowtranssize, int &size);									//패킹 NOW_SIZE
bool Packing(char * buf, int nowtranssize, char * packetbuf, int & size);				//패킹 DATA
bool UnPacking(char * buf, int & nowtranssize);											//언패킹 DONE
bool UnPacking(char * buf, int & nowtranssize, char * packetbuf);						//언패킹 DATA

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

int main(int argc, char* argv[])			//메인이 받는 인자값
		//메인이 받을 인자값의 갯수, 인자값들
		//콘솔환경에서 사용될것
		//드래그 드랍으로도 위에 인자로 보내는게 가능?
{
	int retval;

	

	if(argc < 2){												//전달할 파일명을 명시하지 않았다
		fprintf(stderr, "Usage: %s <FileName>\n", argv[0]);		//파일의 이름을 출력하는것
		return -1;												
	}

	// 윈속 초기화
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

	_DataPacket* ptr=new _DataPacket;						//현재는 하나만 잇으면되니 지역에서 정적으로 선언해도 된다

	ZeroMemory(ptr, sizeof(_DataPacket));

	
	// 파일 열기
	FILE *fp = fopen(argv[1], "rb");						//읽을수있게 바이너리 형태로 파일을 열기

	if(fp == NULL){
		perror("파일 입출력 오류");
		return -1;
	}

	strcpy(ptr->filename, argv[1]);							//파일의 처음부분으로 포인터를 이동
	fseek(fp, 0, SEEK_END);									//파일의 끝으로 이동
	ptr->filesize = ftell(fp);								//ftell 인자값에서부터 파일의 처음부분까지의 크기를 재는 함수
															//즉 fseek(fp, 0, SEEK_END) 와 ptr->filesize = ftell(fp) 를 한 이유는 파일의 전체크기를 알기위함
	int size = 0;
	Packing(ptr->buf, ptr->filename, ptr->filesize, ptr->nowtranssize, size);

	retval = send(sock, (char*)ptr->buf, sizeof(size) + size, 0);
	if (retval == SOCKET_ERROR) err_quit("send()");

	PROTOCOL protocol;
	SizeRecvn(sock, ptr->buf, size);
	if (!PacketRecvn(sock, ptr->buf, size, protocol))
	{
		closesocket(sock);
		printf("서버로부터 연결이 끊어졌습니다");
		WSACleanup();
		return 0;
	}

	if (protocol == NOW_SIZE)
	{
		UnPacking(ptr->buf, ptr->nowtranssize);
	}

	else if (protocol == DONE)
	{
		printf("이미 존재하는 파일입니다.\n");

		closesocket(sock);
		WSACleanup();
		return 0;
	}

	else
	{
		WSACleanup();
		return -1;
	}

	// 파일 데이터 전송에 사용할 변수
	
	int numtotal = 0;										//이곳에 왓다면 파일을 받는곳에 파일이 있거나 없어서 nowtranssize는 0혹은 어떠한 수가 들어가있을것

	fseek(fp, ptr->nowtranssize, SEEK_SET);					//nowtranssize 로 파일의 위치를 옯긴다
	ptr->filesize-=ptr->nowtranssize;						//포인터의 위치도 이동한다


	// 파일 데이터 보내기

	while(1){
		ptr->nowtranssize = fread(ptr->packetbuf, 1, BUFSIZE, fp);			//읽기

		if(ptr->nowtranssize > 0){										//0바이트보다 더많은 값을 읽으는데 성공한다면
			
			Packing(ptr->buf, ptr->nowtranssize, ptr->packetbuf, size);

			retval = send(sock, ptr->buf, sizeof(size) + size, 0);
			if (retval == SOCKET_ERROR) err_quit("send()");

			numtotal += ptr->nowtranssize;
			printf("..");												//중간에 끊는걸 테스트하기위한 부분
			Sleep(100);
		}
		else if(ptr->nowtranssize == 0 && numtotal == ptr->filesize){	//파일의 사이즈가 0이고 파일의 크기와 numtotal 이 같다면 전부다 송신된것
			printf("파일 전송 완료!: %d 바이트\n", numtotal);
			break;
		}
		else{
			perror("파일 입출력 오류");
			break;
		}
	}
	fclose(fp);

	// closesocket()
	closesocket(sock);

	delete ptr;

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