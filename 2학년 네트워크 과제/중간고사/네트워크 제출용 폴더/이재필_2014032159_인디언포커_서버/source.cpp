#pragma comment(lib, "ws2_32")
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#define BUFSIZE 512
#define PLAYERCOUNT 10
#define FULLDECK 20
#define DEAD 1
#define STOP_OVER 2
#define OPENED 3
#define BETTED 4
#define GAMESETED 5
#define QUESTIONSEND 6

enum PROTOCOL { NODATA, INTRO, QUESTION, ANSWER, OPEN, GAMESET, COERCION, PARTNERDIE, SPECIALDIE, DECKMAKING, WAIT };

struct Dealer_info		//딜러의 정보를 저장할 구조체
{
	int Deck[20];		//카드덱을 저장할 20칸의 배열
	int DeckCount;		//카드덱에 남은 카드의 수를 저장할 변수
	int BattingChipSum;	//현재 배팅된 금액의 합을 저장할 변수
};

struct User_Info		//유저의 정보를 저장할 구조체
{
	int Card;			//현재 유저가 들고있는 카드를 저장할 변수
	int Chip;			//현재 유저가 소유한 칩을 저장할 변수
	int BattingChip;	//현재 유저가 배팅한 칩을 저장할 변수
};

struct _ClientInfo					//클라이언트의 정보를 저장할 구조체
{
	SOCKET sock;					//해당 클라이언트의 소켓
	SOCKADDR_IN clientaddr;			//해당 클라이언트의 소켓 주소 구조체
	User_Info * info;				//해당 클라이언트에 저장된 유저의 정보를 저장할 구조체
	Dealer_info * dealer;			//해당 클라이언트의 게임을 담당할 딜러 포인터
	char  packetbuf[BUFSIZE];		//해당 클라이언트 전속 패킷 버퍼
	_ClientInfo* partner;			//해당 클라이언트와 같이 게임을 할 클라이언트를 저장할 포인터
};

_ClientInfo* ClientInfo[PLAYERCOUNT];	//총 10개의 클라이언트 포인터를 미리 준비
int count = 0;							//현재 생성된 클라이언트의 총수

void err_quit(char *msg);																		//에러 출력후 프로그램 종료용 함수
void err_display(char *msg);																	//에러 출력용 함수
int recvn(SOCKET s, char *buf, int len, int flags);												//정확한 크기의 패킷을 받을때 사용할 읽기용 함수
_ClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr);									//새로운 클라이언트를 받을때 사용할 함수
void RemoveClient(_ClientInfo* ptr);															//클라이언트가 나갈때 삭제하기위한 함수
bool MatchPartner(_ClientInfo* _ptr);															//클라이언트 둘을 파트너로 잇기위한 함수
void DeckMaking(Dealer_info * ptr);																//카드를 전부 소모하거나 새로만들어야할 필요가 있을때 사용할 함수
int BattingStart(_ClientInfo* _player);															//배팅이 시작되고 선택권을 가진 유저가 어느걸 선택햇는지 반환할 함수
void StopOver(_ClientInfo* _player);															//만약 send함수를 호출한 결과 클라이언트가 도중에 접속을 끊은걸 알게되었을경우 호출할 함수
bool StringSend(char * _str, _ClientInfo* _player,PROTOCOL _protocol);							//send 함수로 문자열을 보내야할때 가변길이패킷을 만들어낼 함수
DWORD CALLBACK ProcessClient(LPVOID);															//스레드 함수

int main(int argc, char **argv)
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	int retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수		
	int addrlen;
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	_ClientInfo* ptr;
	while (1)
	{
		while (1)
		{
			addrlen = sizeof(clientaddr);
			sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);					//새로운 클라이언트를 받아냅니다
			if (sock == INVALID_SOCKET)
			{
				err_display("accept()");													//에러발생시
				continue;																	//다시 받아냅니다
			}

			ptr = AddClient(sock, clientaddr);												//에러가 발생하지 않을경우 해당 클라이언트를 저장하기위해 AddClient 함수를 호출합니다

			if (MatchPartner(ptr))															//해당 클라이언트와 매치될 플레이어가 있나 보기위해 MatchPartner 함수를 호출합니다
			{
				break;																		//있다면 반복문을 나갈것이고 없다면 못나갈것입니다
			}
		}

		HANDLE hThread = CreateThread(NULL, 0, ProcessClient, ptr, 0, NULL);				//가장 최근에 추가된 클라이언트를 멤버를 인자로 스레드함수를 호출합니다
		if (hThread == NULL)
		{
			RemoveClient(ptr);																//만약 에러가 발생시 해당 클라이언트를 삭제하고
			continue;																		//다시 새로운 클라이언트를 받습니다
		}
		else
		{
			CloseHandle(hThread);															//에러가 발생하지 않을경우 위에서 만든 스레드함수에대한 권한을 포기하여 메인에서 해당 스레드함수를 제어할수 없게합니다
		}
	}																						//서버를 강제적으로 종료하지 않는한 반복문을 계속 돌기때문에 코드상으로 서버가 꺼지지는 않습니다

	closesocket(listen_sock);																//리슨 소켓을을 닫습니다							

	WSACleanup();																			//모든 스레드에서 소켓작업을 중지합니다
	return 0;
}

bool StringSend(char * _str, _ClientInfo* _player, PROTOCOL _protocol)	
			//인자 : 클라이언트에게 보낼 문자열,클라이언트 정보를 담은 _ClientInfo 구조체 포인터,어떤 형식의 패킷인지 구분할 프로토콜
{
	int str_size = strlen(_str);											//전송할 문자열의 길이를 저장합니다
	char * ptr = _player->packetbuf;										//전송할 클라이언트의 버퍼에 데이터를 쓸 준비를 합니다 
	int size = sizeof(_protocol)+sizeof(str_size)+str_size;					//패킷 버퍼의 전체크기를 계산합니다
			//프로토콜의 크기 + 문자열의 길이를 담은 int형 변수의 크기 + 문자열 길이

	switch (_protocol)														//프로토콜로 스위치문을 실행하여
	{
	case QUESTION:															//QUESTION의 경우
		size += sizeof(_player->info->Chip) + sizeof(_player->info->BattingChip) + sizeof(_player->partner->info->Chip);		
		break;						//사이즈 + 플레이어의 칩을 저장한 int형 변수의 크기 + 플레이어가 배팅한 칩을 저장한 int형 변수의 크기 + 플레이어의 파트너의 칩을 저장한 int형 변수의 크기
	
	case OPEN:																//OPEN과
	case PARTNERDIE:														//DECKMAKING과
	case SPECIALDIE:														//SPECIALDIE의 경우
		size += sizeof(_player->info->Chip);								
		break;						//사이즈 + 플레이어의 칩을 저장한 int형 변수의 크기
	}

	memcpy(ptr, &size, sizeof(size));										//포인터의 위치에 사이즈를 복사
	ptr = ptr + sizeof(size);												//포인터 사이즈의 크기만큼 이동

	memcpy(ptr, &_protocol, sizeof(_protocol));								//포인터의 위치에 프로토콜을 복사
	ptr = ptr + sizeof(_protocol);											//포인터 프로토콜의 크기만큼 이동

	memcpy(ptr, &str_size, sizeof(str_size));								//포인터의 위치에 문자열의 크기를 복사
	ptr = ptr + sizeof(str_size);											//포인터 문자열의 크기의 크기만큼 이동

	memcpy(ptr, _str, str_size);											//포인터의 위치에 문자열을 복사
	ptr = ptr + str_size;													//포인터 문자열의 크기만큼 이동

	switch (_protocol)
	{
	case QUESTION:															//QUESTION의 경우 추가로 해당 case문 안에 있는것도 보냅니다
		memcpy(ptr, &_player->info->Chip, sizeof(_player->info->Chip));									//포인터의 위치에 유저의 칩을 복사
		ptr = ptr + sizeof(_player->info->Chip);														//포인터 유저의 칩의 크기만큼 이동

		memcpy(ptr, &_player->info->BattingChip, sizeof(_player->info->BattingChip));					//포인터의 위치에 유저가 배팅한 칩의 량을 복사
		ptr = ptr + sizeof(_player->info->BattingChip);													//포인터 유저가 배팅한 칩의 량의 크기만큼 이동

		memcpy(ptr, &_player->partner->info->BattingChip, sizeof(_player->partner->info->BattingChip));	//포인터의 위치에 유저의 파트너가 배팅한 칩의 량을 복사
		ptr = ptr + sizeof(_player->partner->info->BattingChip);										//포인터 유저의 파트너가 배팅한 칩의 량의 크기만큼 이동
		break;
									
	case OPEN:																//OPEN과
	case PARTNERDIE:														//DECKMAKING과
	case SPECIALDIE:														//SPECIALDIE의 경우 추가로 해당 case문 안에 있는것도 보냅니다
		memcpy(ptr, &_player->info->Chip, sizeof(_player->info->Chip));									//포인터의 위치에 유저의 칩을 복사		
		ptr = ptr + sizeof(_player->info->Chip);														//포인터 유저의 칩의 크기만큼 이동
		break;
	}

	if (send(_player->sock, _player->packetbuf, sizeof(size)+size, 0) == SOCKET_ERROR)	//위에서 만들어진 패킷을 해당 클라이언트에게 보냅니다
	{
		char err_str[BUFSIZE];
		sprintf(err_str, "StringSend()%d", _protocol);									//에러시 나타날 오류문은 프로토콜의 고유숫자로 분별합니다
		err_display(err_str);															//오류시 err_display를 사용하고
		StopOver(_player->partner);														//StopOver로 send에 실패한 유저의 파트너에게 승리 메세지를 보냅니다
		return false;																	//스레드 함수를 종료시키기위해 false를 반환합니다																
	}

	return true;																		//위의 send에서 제대로된 작업을 수행하였다면 여기에서 함수가 끝납니다
}

void StopOver(_ClientInfo* _player)
			//인자 : 승리 처리할 클라이언트 정보를 담은 _ClientInfo 구조체 포인터
{
	char str[] = "상대가 게임을 나갔습니다 당신의 승리입니다.\n";		//클라이언트에게 보낼 문자열
	int str_size = strlen(str);											//문자열의 크기

	char * ptr = _player->packetbuf;									//전송할 클라이언트의 버퍼에 데이터를 쓸 준비를 합니다 
	PROTOCOL protocol = COERCION;										//프로토콜 설정
	int size = sizeof(protocol) + sizeof(str_size) + str_size;			//패킷 버퍼의 전체크기를 계산합니다
			//프로토콜의 크기 + 문자열의 크기를 저장한 int형 변수의 크기 + 문자열의 크기
		
	memcpy(ptr, &size, sizeof(size));									//포인터의 위치에 사이즈를 복사
	ptr = ptr + sizeof(size);											//포인터 사이즈의 크기만큼 이동

	memcpy(ptr, &protocol, sizeof(protocol));							//포인터의 위치에 프로토콜을 복사
	ptr = ptr + sizeof(protocol);										//포인터 프로토콜의 크기만큼 이동

	memcpy(ptr, &str_size, sizeof(str_size));							//포인터의 위치에 문자열의 크기를 복사
	ptr = ptr + sizeof(str_size);										//포인터 문자열의 크기의 크기만큼 이동

	memcpy(ptr, str, str_size);											//포인터의 위치에 문자열을 복사
	ptr = ptr + str_size;												//포인터 문자열의 크기만큼 이동

	send(_player->sock, _player->packetbuf, sizeof(size) + size, 0);	//위에서 만들어진 패킷을 해당 클라이언트에게 보냅니다

	if (_player->dealer != nullptr)										//만약 해당 클라이언트의 담당 딜러가 삭제되지 않았다면
	{
		delete _player->dealer;											//삭제후
		_player->dealer = nullptr;										//nullptr로 초기화합니다
	}

	RemoveClient(_player->partner);										//이 플레이어의 파트너를 삭제하고
	RemoveClient(_player);												//이 클라이언트도 삭제합니다								
}

int BattingStart(_ClientInfo* _player)									
			//인자 : 메뉴 선택권을 가진 클라이언트 정보를 담은 _ClientInfo 구조체 포인터
{
	int size = 0;														//패킷의 크기를 저장할 변수
	PROTOCOL protocol = NODATA;											//패킷의 종류를 저장할 변수
	int retval = 0;														//recvn의 리턴값을 저장할 변수
	char * ptr = nullptr;												//버퍼에 여러가지 값들을 읽어올 포인터 변수

	retval = recvn(_player->sock, (char*)&size, sizeof(size), 0);		//recvn 으로 패킷의 전체크기를 읽어옵니다
	if (retval == SOCKET_ERROR || retval == 0)
	{
		err_display("recv error()1");
		return ERROR;
	}

	retval = recvn(_player->sock, _player->packetbuf, size, 0);			//recvn 으로 읽어온 전체크기만큼 패킷을 읽어옵니다
	if (retval == SOCKET_ERROR || retval == 0)
	{
		err_display("recv error()2");
		return ERROR;
	}

	ptr = _player->packetbuf;											//포인터를 버퍼에 지정해주고
	memcpy(&protocol, ptr, sizeof(protocol));							//protocol에 포인터가 가르키는 값을 복사
	ptr = ptr + sizeof(protocol);										//포인터 protocol의 크기만큼 이동

	if (protocol == ANSWER)												//만약 프로토콜이 ANSWER 일경우에만 아래 문단을 실행
	{
		bool isWantDie;													//클라이언트가 판을 포기했는지 저장할 변수
		bool isWantOpen;												//클라이언트가 오픈을 선택했는지 저장할 변수
		int additionChip;												//클라이언트가 배팅을 선택했을때 추가금을 얼마나 냈는지 저장할 변수

		memcpy(&isWantDie, ptr, sizeof(isWantDie));						//isWantDie에 포인터가 가르키는 값을 복사
		ptr = ptr + sizeof(isWantDie);									//포인터 isWantDie의 크기만큼 이동

		if (!isWantDie)													//만약 클라이언트가 첫번째 질문에서 NO를 선택했을경우 아래 문단을 실행
		{
			if (_player->partner->info->BattingChip > _player->info->BattingChip)				//만약 클라이언트가 배팅한 칩보다 클라이언트의 파트너가 배팅한 칩이 많을경우
			{
				int gap = _player->partner->info->BattingChip - _player->info->BattingChip;		//gap 이라는 변수에 해당 차를 저장

				if (_player->info->Chip - gap < 0)												//만약 가진 칩수 - gap 이 0보다 작을경우
				{
					gap = _player->info->Chip;													//gap은 해당 클라이언트가 가진 칩의 최대치로 설정
				}

				_player->info->Chip -= gap;														//클라이언트가 가진 칩에서 gap 만큼 차감
				_player->info->BattingChip += gap;												//클라이언트가 배팅한 칩에 gap 만큼 합산
				_player->dealer->BattingChipSum += gap;											//클라이언트가 플레이중인 게임의 딜러가 가지고있는 전체 배팅 칩의 량에 gap 만큼 합산
			}

			/*
			if (_player->partner->info->BattingChip > _player->info->BattingChip) 에 대한 설명
			두 플레이어는 번갈아가면서 메뉴를 출력시킬것이고 이과정에서 전에 배팅한 플레이어와
			지금 메뉴 선택권을 가진 플레이어의 사이에는 배팅금의 차이가 발생합니다
			두 플레이어의 배팅금은 메뉴 선택권을 가진 플레이어가 DIE를 선택하지 않을경우 같아야하므로
			위의 연산을 실행합니다

			만약 플레이어가 가진 칩의 량보다 그 차이가 클경우 올인을 하는걸로 그 상황을 넘기는것은 가능합니다
			*/

			memcpy(&isWantOpen, ptr, sizeof(isWantOpen));				//isWantOpen 에 포인터가 가르키는 값을 복사
			ptr = ptr + sizeof(isWantOpen);								//포인터 isWantOpen의 크기만큼 이동

			if (!isWantOpen)											//만약 클라이언트가 두번째 질문에서 NO를 선택했을경우 아래 문단을 실행
			{
				memcpy(&additionChip, ptr, sizeof(additionChip));		//additionChip 에 포인터가 가르키는 값을 복사
				ptr = ptr + sizeof(additionChip);						//포인터 additionChip의 크기만큼 이동

				_player->info->Chip -= additionChip;					//클라이언트가 가진 칩에서 additionChip 만큼 차감
				_player->info->BattingChip += additionChip;				//클라이언트가 배팅한 칩에 additionChip 만큼 합산
				_player->dealer->BattingChipSum += additionChip;		//클라이언트가 플레이중인 게임의 딜러가 가지고있는 전체 배팅 칩의 량에 additionChip 만큼 합산

				return BETTED;											//외부에 해당 클라이언트는 배팅을 선택했다고 알림
			}

			return OPENED;												//외부에 해당 클라이언트는 오픈을 선택했다고 알림
		}

		else
		{
			return DEAD;												//외부에 해당 클라이언트는 다이를 선택했다고 알림
		}

		/*
		이 함수에대한 설명
		이 함수는 두번의 recvn을 하기위한 함수입니다
		첫번째 recvn은 전체 사이즈를 알아오고 끝나지만
		두번째 recvn은 bool변수 2개를 사용하여 총 3종류중 하나의 패킷을 가져옵니다.

		1. 클라이언트가 다이를 선택할경우
		패킷의 내용은 "패킷의 사이즈 | 프로토콜 | DIE를 선택했는지 않았는지 저장할 isWantDie 변수" 
		으로 3개의 내용물을 담고있으며
		이경우 isWantDie 는 true를 가리키고 있게 됩니다

		2. 클라이언트가 오픈을 선택할경우
		패킷의 내용은 "패킷의 사이즈 | 프로토콜 | isWantDie | OPEN을 선택했는지 않았는지 저장할 isWantOpen"
		으로 4개의 내용물을 담고있으며
		이경우 isWantDie 는 false, isWantOpen 는 true를 가리키게 됩니다

		3. 클라이언트가 배팅을 선택할경우
		패킷의 내용은 "패킷의 사이즈 | 프로토콜 | isWantDie | isWantOpen | 추가 배팅금"
		으로 5개의 내용물을 담고있으며
		이경우 isWantDie 는 false, isWantOpen 는 false를 가리키게 됩니다
		*/

	}

	return ERROR;														//이 부분에서 끝나는것은 에러입니다
}

DWORD CALLBACK ProcessClient(LPVOID arg)								//스레드 함수
{	
	int battingResult = QUESTIONSEND;									//BattingStart 함수의 결과를 저장할 변수
	int mainPlayer = 0;													//메뉴출력을 해서 메뉴에대한 선택권을 가진 유저
	bool newCardFlag = true;
	_ClientInfo* player[2];												//두개의 클라이언트 포인터 배열
	player[0] = (_ClientInfo*)arg;										//0번은 인자로온 arg
	player[1] = player[0]->partner;										//1번은 0번의 파트너

	Dealer_info * gameDealer = new Dealer_info;							//게임이 시작되기전 새로운 딜러를 생성
	DeckMaking(gameDealer);												//카드를 생성

	player[0]->dealer = gameDealer;										//0번 플레이어의 담당 딜러를 위에서 만든 딜러로 지정
	player[1]->dealer = gameDealer;										//1번 플레이어의 담당 딜러를 위에서 만든 딜러로 지정

	gameDealer->BattingChipSum = 0;										//현재 배팅칩 합을 0으로 초기화			

	while (1)
	{
		int size = 0;													//패킷의 최종 사이즈를 저장할 변수
		PROTOCOL protocol = NODATA;										//패킷의 종류를 정할 프로토콜 변수
		int retval = 0;													//함수의 리턴값 저장용
		char * ptr = nullptr;											//패킷을 읽을 포인터
		char str[BUFSIZE] = "";											//문자열을 저장할 캐릭터배열
		int str_size = 0;												//문자열의 크기를 저장할 변수

		if (gameDealer->DeckCount == 0)									//만약 카드를 전부 사용했을경우 아래 문단을 실행합니다
		{
			sprintf(str, "\n딜러 : 20장의 카드를 모두 사용하였습니다. \n새로운 덱을 만들겠습니다.\n");
			//이와 같은 문자열을 str에 저장합니다
			for (int i = 0; i < 2; i++)
			{
				if (!StringSend(str, player[i], DECKMAKING))
				{
					return ERROR;
				}					//위의 문자열을 클라이언트에게 전송합니다
			}

			DeckMaking(gameDealer);										//새로운 덱을 만듭니다
		}

		if (newCardFlag)
		{
			for (int i = 0; i < 2; i++)
			{
				player[i]->info->Chip -= 1;												//기본배팅을 위해 두 플레이어의 칩을 1씩 내립니다
				player[i]->info->BattingChip += 1;										//기본배팅을 위해 두 플레이어가 배팅합 칩을 1씩 올립니다
				gameDealer->BattingChipSum += 1;										//기본배팅한 금액의 합을 딜러가 저장해둡니다
				player[i]->info->Card = gameDealer->Deck[gameDealer->DeckCount - 1];	//두 플레이어는 새로운 카드를 받습니다
				gameDealer->DeckCount--;												//딜러는 소유하고잇는 카드의 수를 줄입니다
			}

			newCardFlag = false;
		}

		for (int i = 0; i < 2; i++)														//2번 돌리는 반복문을 준비하고
		{
			sprintf(str, "\n상대의 카드 : %d \n현재 당신의 칩 : %d \n현재 상대의 칩 : %d \n현재 당신이 배팅한 칩 : %d \n현재 상대가 배팅한 칩 : %d \n현재 배팅된 칩의 합 : %d \n",
				player[i]->partner->info->Card, player[i]->info->Chip, player[i]->partner->info->Chip, player[i]->info->BattingChip, player[i]->partner->info->BattingChip, gameDealer->BattingChipSum);
			str_size = strlen(str);														//위와 같은 문자열을 str에 저장합니다

			ptr = player[i]->packetbuf;													//포인터는 i번째 플레이어의 버퍼
			protocol = INTRO;															//프로토콜은 인트로로 설정합니다.
			size = sizeof(protocol) + sizeof(str_size) + str_size + sizeof(player[i]->info->Chip) + sizeof(player[i]->info->BattingChip) + sizeof(player[i]->partner->info->BattingChip);
				//사이즈는 프로토콜의 크기 + 문자열의 크기를 저장한 변수의 크기 + 문자열의 크기 + i 번째 플레이어의 칩수를 저장한 변수의 크기 
				//+ i 번째 플레이어가 배팅한 칩을 저장한 변수의 크기 + i 번째 플레이어의 파트너가 배팅한 칩을 저장한 변수의 크기 입니다

			memcpy(ptr, &size, sizeof(size));																		//포인터의 위치에 사이즈를 복사
			ptr = ptr + sizeof(size);																				//포인터 사이즈의 크기만큼 이동

			memcpy(ptr, &protocol, sizeof(protocol));																//포인터의 위치에 프로토콜을 복사
			ptr = ptr + sizeof(protocol);																			//포인터 프로토콜의 크기만큼 이동

			memcpy(ptr, &str_size, sizeof(str_size));																//포인터의 위치에 문자열의 크기를 복사
			ptr = ptr + sizeof(str_size);																			//포인터 문자열의 크기의 크기만큼 이동

			memcpy(ptr, str, str_size);																				//포인터의 위치에 문자열을 복사
			ptr = ptr + str_size;																					//포인터 문자열의 크기만큼 이동

			memcpy(ptr, &player[i]->info->Chip, sizeof(player[i]->info->Chip));										//포인터의 위치에 플레이어의 칩수를 복사
			ptr = ptr + sizeof(player[i]->info->Chip);																//포인터 플레이어의 칩수의 크기만큼 이동

			memcpy(ptr, &player[i]->info->Chip, sizeof(player[i]->info->BattingChip));								//포인터의 위치에 플레이어가 배팅한 칩을 복사
			ptr = ptr + sizeof(player[i]->info->BattingChip);														//포인터 플레이어가 배팅한 칩의 크기만큼 이동

			memcpy(ptr, &player[i]->partner->info->BattingChip, sizeof(player[i]->partner->info->BattingChip));		//포인터의 위치에 플레이어의 파트너가 배팅한 칩을 복사
			ptr = ptr + sizeof(player[i]->partner->info->BattingChip);												//포인터 플레이어의 파트너가 배팅한 칩의 크기만큼 이동

			if (send(player[i]->sock, player[i]->packetbuf, sizeof(size) + size, 0) == SOCKET_ERROR)				//위에서 만든 패킷을 send합니다
			{
				err_display("IntroSend()");																				//오류시 이런 문자열을 뛰울것이며
				StopOver(player[i]->partner);																			//파트너에게 승리메세지를 보냅니다
				return STOP_OVER;																						//함수를 종료시킵니다
			}
		}

		if (battingResult == QUESTIONSEND)																				//이번턴에 메뉴 선택권를 가진 플레이어에게 메뉴를 출력하기위한 if문입니다
		{
			sprintf(str, "상대가 선택권을 가지고있습니다.\n");															//이러한 문자열을
			if (!StringSend(str, player[mainPlayer]->partner, WAIT))													//WAIT포로토콜과 함께 선택권을 가진 유저의 파트너에게 보내고
			{
				return STOP_OVER;
			}
			
			sprintf(str, "Die ? 1. yes, 2. no \nAnswer :");																//이러한 문자열을
			if (!StringSend(str, player[mainPlayer], QUESTION))															//QUESTION 프로토콜과 함께 메뉴선택권을 가진 유저에게 보냅니다 
			{
				return STOP_OVER;
			}
			battingResult = BattingStart(player[mainPlayer]);															//그리고 BattingStart 함수를 실행하여 결과를 battingResult 에 저장합니다
		}

		if (battingResult == DEAD)																							//만약 BattingStart의 결과가 DEAD 였을경우
		{
			if (player[mainPlayer]->info->Card == 10)																		//만약 해당 플레이어가 가지고있던 카드가 10이였을경우
			{
				player[mainPlayer]->partner->info->Chip += 10;															
				player[mainPlayer]->info->Chip -= 10;																		//파트너에게 10개의 칩을 빼앗깁니다

				sprintf(str, "\n상대가 DIE를 선택했습니다. \n상대가 10을 가지고 패배했으므로 10개의 칩을 받습니다.\n");		//이러한 문자열을
				if (!StringSend(str, player[mainPlayer]->partner, SPECIALDIE))												//SPECIALDIE 프로토콜과 함께 해당 플레이어의 파트너에게 보내고
				{
					return STOP_OVER;
				}

				sprintf(str, "\n당신은 10을 가지고 패배하였습니다. \n상대에게 10개의 칩을 줍니다.\n");						//이러한 문자열을
				if (!StringSend(str, player[mainPlayer], SPECIALDIE))														//SPECIALDIE 프로토콜과 함께 해당 플레이어에게 보냅니다
				{
					return STOP_OVER;
				}
			}

			else																											//그외의 카드를 가지고 포기할경우
			{
				sprintf(str, "\n상대가 DIE를 선택했습니다. \n상대의 카드 였던것 : %d\n", player[mainPlayer]->info->Card);	//두 클라이언트 모두 자신의 카드를 보여주지 않고 끝납니다
				if (!StringSend(str, player[mainPlayer]->partner, PARTNERDIE))												//위의 문자열과 PARTNERDIE 프로토콜을 해당 플레이어의 파트너에게 보냅니다
				{
					return STOP_OVER;
				}
			}

			player[mainPlayer]->info->BattingChip = 0;																	//메뉴 선택권을 가진 플레이어가 배팅했던 칩을 0으로 만들고
			player[mainPlayer]->partner->info->BattingChip = 0;															//그 파트너가 배팅했던 칩을 0으로 만듭니다
			player[mainPlayer]->partner->info->Chip += gameDealer->BattingChipSum;										//그후 파트너는 딜러가 저장하고있던 배팅칩의 합계를 자신이 가지고있던 전체칩과 합산합니다
			gameDealer->BattingChipSum = 0;																				//마지막으로 딜러가 가지고있던 두 플레이어의 배팅칩의 합계를 0으로만듭니다
			newCardFlag = true;
		}

		mainPlayer = (mainPlayer == 1) ? 0 : 1;																			//메뉴 선택권을 가진 플레이어를 바꿉니다

		if (battingResult == OPENED)																					//만약 BattingStart의 결과가 OPENED 였을경우
		{
			for (int i = 0; i < 2; i++)																					//두플레이어에게 출력해주기위해 2번돌 반복문을 준비하고
			{
				//승리
				if (player[i]->info->Card > player[i]->partner->info->Card)													//만약 i번째 플레이어가 가지고있는 카드가 파트너의 카드보다 크다면
				{
					player[i]->info->BattingChip = 0;																		//i번째 플레이어가 배팅한 칩을 0으로 초기화해줍니다
					player[i]->info->Chip += gameDealer->BattingChipSum;													//해당 플레이어의 칩에 딜러가 가지고있던 배팅칩의 합계를 합산합니다
					gameDealer->BattingChipSum = 0;																			//그후 딜러가 가지고있던 배팅칩의 합계를 0으로 초기화합니다
					sprintf(str, "\n오픈이 선택되었습니다. \n당신의 카드 : %d, 상대의 카드 : %d \n승리입니다.\n현재 당신의 칩 : %d\n", player[i]->info->Card, player[i]->partner->info->Card, player[i]->info->Chip);
					mainPlayer = i;																							//승자에게 위의 문자열을 저장하고 다음판의 메뉴 선택권을 승자에게 줍니다
				}

				//패배
				else if (player[i]->info->Card < player[i]->partner->info->Card)											//만약 i번째 플레이어가 가지고있는 카드가 파트너의 카드보다 작다면
				{
					player[i]->info->BattingChip = 0;																		//i번째 플레이어가 배팅한 칩을 0으로 초기화해줍니다
					sprintf(str, "\n오픈이 선택되었습니다. \n당신의 카드 : %d, 상대의 카드 : %d \n패배입니다.\n현재 당신의 칩 : %d\n", player[i]->info->Card, player[i]->partner->info->Card, player[i]->info->Chip);
				}																											//위의 문자열을 저장합니다	

				//무승부
				else																										//만약 i번째 플레이어가 가지고있는 카드가 파트너의 카드하고 똑같다면
				{
					sprintf(str, "\n오픈이 선택되었습니다. \n당신의 카드 : %d, 상대의 카드 : %d \n무승부입니다.\n현재 당신의 칩 : %d\n배팅된 칩은 다음판에 사용됩니다.\n", player[i]->info->Card, player[i]->partner->info->Card, player[i]->info->Chip);
				}																											//위의 문자열을 저장합니다. 그리고 딜러가 보유한 배팅칩의 합계는 0으로 초기화되지 않으며 다음판에 사용됩니다

				if (!StringSend(str, player[i], OPEN))																		//위에서 저장한 문자열을 OPEN 프로토콜과 함께 i번째 플레이어에게 보냅니다
				{
					return STOP_OVER;
				}
				newCardFlag = true;																							//오픈을 하엿으니 새로운 카드를 주기위해 플래그를 세웁니다
			}
		}

		battingResult = QUESTIONSEND;																					//한 플레이어가 메뉴에서 어떠한것을 선택하였고 또다시 메뉴 선택권을 가진 플레이어에게 메뉴를 출력시켜줘야하므로 QUESTIONSEND로 battingResult 를 초기화해줍니다

		for (int i = 0; i < 2; i++)																						//2번 검사하기위해 2번돌 만복문을 준비해주고
		{
			if (player[i]->info->Chip <= 0 && player[i]->info->BattingChip == 0 && gameDealer->BattingChipSum == 0)		//i 번째 플레이어의 칩이 0이하이며 배팅한 칩이 0개이며 딜러가 소유한 배팅칩의 합계가 0개일때
			{
				sprintf(str, "당신은 칩을 전부 소진했습니다. 패배하였습니다.\n");										//해당 플레이어는 패배하였으므로 이러한 문자열을
				if (!StringSend(str, player[i], GAMESET))																//GAMESET 프로토콜과 함께 보내줍니다
				{
					return STOP_OVER;
				}

				sprintf(str, "상대가 칩을 전부 소진했습니다. 승리하였습니다.\n");										//그리고 그 플레이어의 파트너에게 이러한 문자열을
				if (!StringSend(str, player[i]->partner, GAMESET))														//GAMESET 프로토콜과 함께 보내줍니다
				{
					return STOP_OVER;
				}

				if (gameDealer != nullptr)																				//딜러가 아직 삭제가 안되었다면
				{
					delete gameDealer;																					//딜러를 삭제하고
					gameDealer = nullptr;																				//nullptr으로 대입해줍니다
				}

				RemoveClient(player[i]->partner);																		//그후 파트너를 우선적으로 삭제해주고
				RemoveClient(player[i]);																				//i 번째 플레이어를 삭제합니다

				return GAMESETED;
			}
		}
	}
	return 0;
}

void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
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

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

_ClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr)
					//인자 : 소켓,소켓 구조체
{
	_ClientInfo* ptr = new _ClientInfo;				//새로운 클라이언트 구조체 동적할당
	ZeroMemory(ptr, sizeof(_ClientInfo));			//해당 동적할당 구조체 초기화
	ptr->sock = sock;								//소켓 저장
	memcpy(&(ptr->clientaddr), &clientaddr, sizeof(clientaddr));
													//소켓 구조체 복사
	ptr->info = new User_Info;						//클라이언트 속 유저정보 구조체 동적할당
	ptr->info->BattingChip = 0;						//유저가 배팅한 칩의 량 0으로 초기화
	ptr->info->Card = 0;							//유저가 들고있는 카드 0으로 초기화
	ptr->info->Chip = 30;							//유저가 가지고있는 칩 30개로 초기화

	ClientInfo[count] = ptr;						//ClientInfo 배열의 count 번쨰에 해당 클라이언트 정보 저장
	printf("\nClient 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));				//서버에서 출력
	count++;										//count 증가

	return ptr;
}

void RemoveClient(_ClientInfo* ptr)
				//인자 : 삭제할 _ClientInfo 구조체의 포인터 
{
	closesocket(ptr->sock);								//해당 클라이언트의 소켓을 닫습니다

	printf("\nClient 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(ptr->clientaddr.sin_addr),
		ntohs(ptr->clientaddr.sin_port));				//서버에서 출력

	for (int i = 0; i < count; i++)						//count만큼 돌 반복문으로
	{
		if (ClientInfo[i] == ptr)						//ClientInfo에 해당 클라이언트가 있는지 확인
		{
			if (ptr->partner != nullptr)				//삭제할 클라이언트의 파트너가 널포인터가 아닌경우
			{
				ptr->partner->partner = nullptr;		//삭제할 클라이언트의 파트너가 자신을 가르키는걸 널포인터를 가르키게 바꿔줌
			}

			delete ptr->info;							//삭제할 클라이언트속 유저정보를 삭제
			delete ptr;									//해당 클라이언트 삭제

			int j;
			for (j = i; j < count - 1; j++)				//count - 1 만큼 돌려서
			{
				ClientInfo[j] = ClientInfo[j + 1];		//삭제한 클라이언트의 위치에서부터 뒤에있는 클라이언트들을 옯겨줌
			}
			ClientInfo[j] = nullptr;					//마지막 클라이언트 nullptr
			break;
		}
	}

	count--;											//삭제 완료후 count 1감소

}

bool MatchPartner(_ClientInfo* _ptr)
			//인자 : 파트너를 찾아야하는 _ClientInfo 구조체의 포인터
{
	for (int i = 0; i < count; i++)											//count 만큼 돌릴 반복문
	{
		if (_ptr != ClientInfo[i] && ClientInfo[i]->partner == nullptr)		//만약 파트너가 없는 클라이언트가 있고 그게 자신이 아닐경우에만
		{
			ClientInfo[i]->partner = _ptr;
			_ptr->partner = ClientInfo[i];									//서로 파트너 연결
			return true;													//성공 반환
		}
	}

	return false;															//성공을 반환하지 못할경우 실패 반환
}

void DeckMaking(Dealer_info * ptr)
				//인자 : 새로운 덱을 만들어야하는 Dealer_info 포인터
{
	srand((unsigned)time(NULL));					

	for (int j = 0; j < 10; j++)					//10번 돌릴 반복문을 준비
	{
		ptr->Deck[j] = j + 1;						//j번째에 j+1을 넣고
		ptr->Deck[j + 10] = j + 1;					//마찬가지로 j+10의 위치에도 j+1을 넣습니다.
	}												//카드는 1~10이 각각 2장씩 있기때문입니다.

	int ShuffleCount = (rand() % 20) + 30;			//최소 30 ~ 최대 49 의 랜덤 수 생성

	for (int i = 0; i < ShuffleCount; i++)			//ShuffleCount 만큼 반복
	{
		int randA;									//랜덤수 저장용 1
		int randB;									//랜덤수 저장용 2
		int temp;									//스왑시 임시저장소

		while (1)
		{
			randA = (rand() % 20);					//최소 0 ~ 최대 19
			randB = (rand() % 20);					//의 랜덤수를 두개 생성

			if (randA != randB)						//같은 수가 나온다면 다시생성
			{
				break;								//아니라면 반복문 탈출
			}
		}

		temp = ptr->Deck[randA];					//randA 번째 카드와
		ptr->Deck[randA] = ptr->Deck[randB];		//randB 번째 카드의
		ptr->Deck[randB] = temp;					//위치를 바꿉니다
	}

	ptr->DeckCount = FULLDECK;						//카드가 가득찼다는걸 알리기위해 FULLDECK 으로 초기화
}