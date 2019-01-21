/*
 * echo_client_win.c
 * Written by SW. YOON
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUFSIZE 512
#define SERVERDROP 1
enum PROTOCOL { NODATA, INTRO, QUESTION, ANSWER, OPEN, GAMESET, COERCION, PARTNERDIE, SPECIALDIE, DECKMAKING, WAIT };
					
struct ClaData													//클라이언트에서 필요한 정보를 저장한 구조체
{
	int Chip;													//현재 가진 칩수
	int BattingChip;											//현재 자신이 배팅한 칩수
	int PartnerBattingChip;										//현재 파트너가 배팅한 칩수
};

void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);

int main(int argc, char **argv)
{
  WSADATA wsaData;

  int result;
  SOCKET hSocket;
  SOCKADDR_IN servAddr;

  ClaData * info = new ClaData;
  info->Chip = 30;

  if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */
		err_quit("WSAStartup() error!"); 

  hSocket=socket(PF_INET, SOCK_STREAM, 0);   
  if(hSocket == INVALID_SOCKET)
    err_quit("socket() error");

  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family=AF_INET;
  servAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
  servAddr.sin_port=htons(9000);

  if(connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr))==SOCKET_ERROR)
    err_quit("connect() error!");

  bool isBaseBet = true;										//메세지 출력할지 안할지 정할 변수

  printf("\n\n잠시 기다려주세요\n\n");							

  while (1)
  {
	  if (isBaseBet)
	  {
		  printf("1개의 칩을 기본배팅합니다\n\n");				
		  isBaseBet = false;									//다시 출력할수없게 false로 바꿉니다
																//PARTNERDIE 혹은 OPEN 혹은 SPECIALDIE 의 세가지 프로토콜을 받으면 다시 true로 변합니다
	  }

	  char buf[BUFSIZE];										//클라이언트의 버퍼
	  int size = 0;												//패킷의 최종 사이즈
	  char str[BUFSIZE] = "";									//문자열을 저장할 배열
	  int str_size = 0;											//문자열의 길이를 저장할 변수
	  PROTOCOL protocol = NODATA;								//패킷의 종류를 정할 프로토콜
	  bool ExitFlag = false;									//프로그램 종료 플래그

	  result = recvn(hSocket, (char*)&size, sizeof(size), 0);	//클라이언트는 기본적으로 서버에서 무언가를 보내기전에는 동결합니다
	  if (result == SOCKET_ERROR || result == 0)				//일단 무언가를 받을수 있게된다면 그 패킷의 전체 크기를 먼저 받아봅니다
	  {
		  err_display("recv error()1");
		  return -1;
	  }

	  result = recvn(hSocket, buf, size, 0);					//그리고 그 패킷을 전부 받은후
	  if (result == SOCKET_ERROR || result == 0)
	  {
		  err_display("recv error()2");
		  return -1;
	  }

	  char* ptr = buf;											//패킷 읽기용 포인터를 만들어주고
	  memcpy(&protocol, ptr, sizeof(protocol));					//우선 프로토콜을 먼저 읽어봅니다
	  ptr = ptr + sizeof(protocol);								//그후 프로토콜을 읽은만큼 포인터를 이동시킵니다

	  bool isWantDie = true;										//클라이언트가 다이를 선택했는지 안했는지 저장할 변수
	  bool isWantOpen = true;										//클라이언트가 오픈을 선택했는지 안했는지 저장할 변수
	  int additionChip = 0;											//클라이언드가 얼마나 더 추가금을 냈는지 저장할 변수

	  switch (protocol)											//프로토콜로 스위치문을 실행시킨후
	  {
	  case INTRO:												//INTRO의 경우
		  {
			  //------------------------------------------------------------------------------
			  //INTRO 받음

			  memcpy(&str_size, ptr, sizeof(str_size));										//str_size 에 문자열의 길이를 복사
			  ptr = ptr + sizeof(str_size);													//str_size 의 크기만큼 포인터 이동

			  memcpy(str, ptr, str_size);													//str 에 문자열 복사
			  ptr = ptr + str_size;															//str_size 만큼 포인터 이동

			  memcpy(&info->Chip, ptr, sizeof(info->Chip));									//info->Chip 에 유저의 칩의 개수 복사
			  ptr = ptr + sizeof(info->Chip);												//info->Chip의 크기만큼 포인터 이동

			  memcpy(&info->BattingChip, ptr, sizeof(info->BattingChip));					//info->BattingChip 에 유저가 배팅한 칩의 개수 복사
			  ptr = ptr + sizeof(info->BattingChip);										//info->BattingChip의 크기만큼 포인터 이동

			  memcpy(&info->PartnerBattingChip, ptr, sizeof(info->PartnerBattingChip));		//info->PartnerBattingChip 에 파트너가 배팅한 칩의 개수 복사
			  ptr = ptr + sizeof(info->PartnerBattingChip);									//info->PartnerBattingChip의 크기만큼 포인터 이동

																							//여기서 info->Chip, info->BattingChip, info->PartnerBattingChip 를 받은 이유는
																							//초기화입니다

			  str[str_size] = '\0';															//문자열 끝에 널문자 붙히기
			  printf("%s", str);															//출력
			  break;																		//switch case문 종료
		  }
      case QUESTION:												//QUESTION의 경우
		  {
			  //------------------------------------------------------------------------------
			  //QUESTION 받음

			  memcpy(&str_size, ptr, sizeof(str_size));										//str_size 에 문자열의 길이를 복사
			  ptr = ptr + sizeof(str_size);													//str_size 의 크기만큼 포인터 이동

			  memcpy(str, ptr, str_size);													//str 에 문자열 복사
			  ptr = ptr + str_size;															//str_size 만큼 포인터 이동

			  memcpy(&info->Chip, ptr, sizeof(info->Chip));									//info->Chip 에 유저의 칩의 개수 복사
			  ptr = ptr + sizeof(info->Chip);												//info->Chip의 크기만큼 포인터 이동

			  memcpy(&info->BattingChip, ptr, sizeof(info->BattingChip));					//info->BattingChip 에 유저가 배팅한 칩의 개수 복사
			  ptr = ptr + sizeof(info->BattingChip);										//info->BattingChip의 크기만큼 포인터 이동

			  memcpy(&info->PartnerBattingChip, ptr, sizeof(info->PartnerBattingChip));		//info->PartnerBattingChip 에 파트너가 배팅한 칩의 개수 복사
			  ptr = ptr + sizeof(info->PartnerBattingChip);									//info->PartnerBattingChip의 크기만큼 포인터 이동
																							
																							//여기서 info->Chip, info->BattingChip, info->PartnerBattingChip 를 받은 이유는
																							//서버에서는 계속되는 다이, 오픈, 배팅으로 저 수치가 계속 변동되지만
																							//이시점에서 보내지 않는다면 클라이언트에서는 이를 저장할 방법이 없습니다

			  str[str_size] = '\0';															//받은 문자열 뒤에 널문자를 붙힙니다

			  //------------------------------------------------------------------------------
			  //QUESTION 출력

			  while (1)
			  {
				  int c;
				  printf("%s", str);														//위에서 받은 문자열을 출력합니다
				  scanf("%d", &c);															//그리고 입력을 받고
				  if (c == 1 || c == 2)
				  {
					  isWantDie = (c == 2) ? false : true;									//선택에따라 isWantDie의 값을 변동시킵니다
					  break;
				  }
			  }
			  //------------------------------------------------------------------------------
			  //ANSWER 보냄

			  ptr = buf;																	//버퍼에 쓸수있게 포인터를 초기화해주고
			  protocol = ANSWER;															//프로토콜을 ANSWER 로 설정해줍니다
			  size = sizeof(protocol) + sizeof(isWantDie);									//사이즈를 정합니다
					//프로토콜의 크리 + 죽을지 안죽을지 정한 bool 변수

			  //----------------------다이를 선택한다면 여기서 size 스탑----------------------

			  if (!isWantDie)																//다이를 선택하지 않을경우
			  {
				  if (info->PartnerBattingChip > info->BattingChip)							//칩의 갯수가 차이나면 그 차이를 매꾸는 부분입니다
				  {
					  int gap = info->PartnerBattingChip - info->BattingChip;					//gap에 파트너가 건 칩의 갯수와 자신이 건 칩의 갯수의 차를 저장합니다

					  if (info->Chip - gap < 0)													//만약 남은 칩에서 gap을 뺀값이 0보다 작을경우
					  {
						  gap = info->Chip;														//gap은 남은 칩의 개수와 똑같아집니다
					  }

					  if (gap != 0)																//gap이 0이 아닐때만
					  {
						  printf("\n딜러 : %d개의 칩이 차이가 납니다 매꾸겠습니다.\n", gap);	//이러한 메세지를 띄웁니다
					  }

					  info->Chip -= gap;														//가진칩에서 gap을 차감하고
					  info->BattingChip += gap;													//배팅한칩에 gap을 합산합니다

					  printf("남은 칩 : %d.\n", info->Chip);									//게산하고 남은칩을 보여줍니다
				  }

				  if (info->Chip > 0)														 //만약 플레이어의 남은 칩이 0개 이하라면 묻지않고 오픈합니다
				  {
					  while (1)
					  {
						  int c;
						  printf("\n1. 오픈 2. 배팅\n");									 //오픈할지 배팅할지 묻는부분입니다
						  printf("Answer : ");
						  scanf("%d", &c);

						  if (c == 1 || c == 2)
						  {
							  isWantOpen = (c == 2) ? false : true;							 //위에서 선택한 값에 따라 isWantOpen의 값이 변동됩니다
							  break;
						  }
					  }
				  }

				  size += sizeof(isWantOpen);												 //그리고 size에 isWantOpen의 크기를 더합니다

				  //----------------------오픈을 선택한다면 여기서 size 스탑----------------------

				  
				  if (!isWantOpen)															 //만약 오픈을 하지 않는경우 
				  {
					  while (1)
					  {
						  printf("\n남은 칩 : %d\n", info->Chip);
						  printf("얼마의 칩을 더 배팅하겠습니까? : ");						 //배팅을 얼마나 더할지 묻고
						  scanf("%d", &additionChip);										 

						  //가진칩이하만 입력가능
						  if (info->Chip >= additionChip && additionChip > 0)				 //정확한 값을 입력했다면
						  {
							  info->Chip - additionChip;
							  info->BattingChip += additionChip;
							  break;
						  }
					  }

					  size += sizeof(additionChip);											 //size에 additionChip의 크기를 더합니다

					  //----------------------배팅을 선택한다면 여기서 size 스탑----------------------
				  }
			  }

			  memcpy(ptr, &size, sizeof(size));											//포인터의 위치에 사이즈를 복사
			  ptr = ptr + sizeof(size);													//포인터 사이즈의 크기만큼 이동

			  memcpy(ptr, &protocol, sizeof(protocol));									//포인터의 위치에 프로토콜을 복사
			  ptr = ptr + sizeof(protocol);												//포인터 프로토콜의 크기만큼 이동

			  memcpy(ptr, &isWantDie, sizeof(isWantDie));								//포인터의 위치에 isWantDie를 복사
			  ptr = ptr + sizeof(isWantDie);											//포인터 isWantDie의 크기만큼 이동

			  //----------------------다이를 선택했다면 여기까지 send()----------------------

			  if (!isWantDie)																//만약 isWantDie 가 false 일경우
			  {		
				  memcpy(ptr, &isWantOpen, sizeof(isWantOpen));								//포인터의 위치에 isWantOpen를 복사
				  ptr = ptr + sizeof(isWantOpen);											//포인터 isWantOpen의 크기만큼 이동

				  //----------------------오픈를 선택했다면 여기까지 send()----------------------

				  if (!isWantOpen)																//만약 isWantOpen 가 false 일경우
				  {
					  memcpy(ptr, &additionChip, sizeof(additionChip));							//포인터의 위치에 추가배팅금을 복사
					  ptr = ptr + sizeof(additionChip);											//포인터 추가배팅금의 크기만큼 이동

					  //----------------------배팅를 선택했다면 여기까지 send()----------------------
				  }
			  }

			  if (send(hSocket, buf, sizeof(size) + size, 0) == SOCKET_ERROR)			//위에서 만든 패킷을 서버에게 송신합니다
			  {
				  err_display("DieSend()");
				  return SERVERDROP;
			  }

			  break;
		  }
	  case WAIT:
	  case DECKMAKING:												//DECKMAKING의 경우
		  {
			  memcpy(&str_size, ptr, sizeof(str_size));										//str_size 에 문자열의 길이를 복사
			  ptr = ptr + sizeof(str_size);													//str_size 의 크기만큼 포인터 이동

			  memcpy(str, ptr, str_size);													//str 에 문자열 복사
			  ptr = ptr + str_size;															//str_size 만큼 포인터 이동

			  str[str_size] = '\0';															//문자열 끝에 널문자

			  printf("%s", str);															//출력
			  break;
		  }

	  case OPEN:													//OPEN과
	  case PARTNERDIE:												//DECKMAKING과
	  case SPECIALDIE:												//SPECIALDIE의 경우
		  {
			  memcpy(&str_size, ptr, sizeof(str_size));										//str_size 에 문자열의 길이를 복사
			  ptr = ptr + sizeof(str_size);													//str_size 의 크기만큼 포인터 이동

			  memcpy(str, ptr, str_size);													//str 에 문자열 복사
			  ptr = ptr + str_size;															//str_size 만큼 포인터 이동

			  memcpy(&info->Chip, ptr, sizeof(info->Chip));									//info->Chip 에 현재 유저의 칩수를 저장
			  ptr = ptr + sizeof(info->Chip);												//info->Chip의 크기만큼 포인터 이동

			  info->BattingChip = 0;														//그후 배팅한 칩을 0으로 초기화하고
			  info->PartnerBattingChip = 0;													//상대하 배팅한 칩도 0으로 초기화합니다

			  str[str_size] = '\0';															//문자열 끝에 널문자
			  printf("%s", str);															//출력

			  isBaseBet = true;																//다시한번 기본배팅 메세지를 띄우기위해 isBaseBet를 true로 바꿉니다
			  break;
		  }

	  case GAMESET:													//GAMESET과
	  case COERCION:												//COERCION의 경우
		  {
			  memcpy(&str_size, ptr, sizeof(str_size));										//str_size 에 문자열의 길이를 복사
			  ptr = ptr + sizeof(str_size);													//str_size 의 크기만큼 포인터 이동

			  memcpy(str, ptr, str_size);													//str 에 문자열 복사
			  ptr = ptr + str_size;															//str_size 만큼 포인터 이동

			  str[str_size] = '\0';															//문자열 끝에 널문자

			  printf("%s", str);															//출력
			  ExitFlag = true;																//종료 플래그를 세우고
			  break;
		  }
	  }

	  if (ExitFlag)																			//게임이 끝났으므로 클라이언트를 종료합니다
	  {
		  break;
	  }
  }  

  closesocket(hSocket);																		//소켓을 닫고
  WSACleanup();																				//모든 스레드에서 소켓작업을 중지합니다
  return 0;
}

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