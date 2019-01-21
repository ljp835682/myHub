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

struct Dealer_info		//������ ������ ������ ����ü
{
	int Deck[20];		//ī�嵦�� ������ 20ĭ�� �迭
	int DeckCount;		//ī�嵦�� ���� ī���� ���� ������ ����
	int BattingChipSum;	//���� ���õ� �ݾ��� ���� ������ ����
};

struct User_Info		//������ ������ ������ ����ü
{
	int Card;			//���� ������ ����ִ� ī�带 ������ ����
	int Chip;			//���� ������ ������ Ĩ�� ������ ����
	int BattingChip;	//���� ������ ������ Ĩ�� ������ ����
};

struct _ClientInfo					//Ŭ���̾�Ʈ�� ������ ������ ����ü
{
	SOCKET sock;					//�ش� Ŭ���̾�Ʈ�� ����
	SOCKADDR_IN clientaddr;			//�ش� Ŭ���̾�Ʈ�� ���� �ּ� ����ü
	User_Info * info;				//�ش� Ŭ���̾�Ʈ�� ����� ������ ������ ������ ����ü
	Dealer_info * dealer;			//�ش� Ŭ���̾�Ʈ�� ������ ����� ���� ������
	char  packetbuf[BUFSIZE];		//�ش� Ŭ���̾�Ʈ ���� ��Ŷ ����
	_ClientInfo* partner;			//�ش� Ŭ���̾�Ʈ�� ���� ������ �� Ŭ���̾�Ʈ�� ������ ������
};

_ClientInfo* ClientInfo[PLAYERCOUNT];	//�� 10���� Ŭ���̾�Ʈ �����͸� �̸� �غ�
int count = 0;							//���� ������ Ŭ���̾�Ʈ�� �Ѽ�

void err_quit(char *msg);																		//���� ����� ���α׷� ����� �Լ�
void err_display(char *msg);																	//���� ��¿� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags);												//��Ȯ�� ũ���� ��Ŷ�� ������ ����� �б�� �Լ�
_ClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr);									//���ο� Ŭ���̾�Ʈ�� ������ ����� �Լ�
void RemoveClient(_ClientInfo* ptr);															//Ŭ���̾�Ʈ�� ������ �����ϱ����� �Լ�
bool MatchPartner(_ClientInfo* _ptr);															//Ŭ���̾�Ʈ ���� ��Ʈ�ʷ� �ձ����� �Լ�
void DeckMaking(Dealer_info * ptr);																//ī�带 ���� �Ҹ��ϰų� ���θ������� �ʿ䰡 ������ ����� �Լ�
int BattingStart(_ClientInfo* _player);															//������ ���۵ǰ� ���ñ��� ���� ������ ����� �����޴��� ��ȯ�� �Լ�
void StopOver(_ClientInfo* _player);															//���� send�Լ��� ȣ���� ��� Ŭ���̾�Ʈ�� ���߿� ������ ������ �˰ԵǾ������ ȣ���� �Լ�
bool StringSend(char * _str, _ClientInfo* _player,PROTOCOL _protocol);							//send �Լ��� ���ڿ��� �������Ҷ� ����������Ŷ�� ���� �Լ�
DWORD CALLBACK ProcessClient(LPVOID);															//������ �Լ�

int main(int argc, char **argv)
{
	// ���� �ʱ�ȭ
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

	// ������ ��ſ� ����� ����		
	int addrlen;
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	_ClientInfo* ptr;
	while (1)
	{
		while (1)
		{
			addrlen = sizeof(clientaddr);
			sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);					//���ο� Ŭ���̾�Ʈ�� �޾Ƴ��ϴ�
			if (sock == INVALID_SOCKET)
			{
				err_display("accept()");													//�����߻���
				continue;																	//�ٽ� �޾Ƴ��ϴ�
			}

			ptr = AddClient(sock, clientaddr);												//������ �߻����� ������� �ش� Ŭ���̾�Ʈ�� �����ϱ����� AddClient �Լ��� ȣ���մϴ�

			if (MatchPartner(ptr))															//�ش� Ŭ���̾�Ʈ�� ��ġ�� �÷��̾ �ֳ� �������� MatchPartner �Լ��� ȣ���մϴ�
			{
				break;																		//�ִٸ� �ݺ����� �������̰� ���ٸ� ���������Դϴ�
			}
		}

		HANDLE hThread = CreateThread(NULL, 0, ProcessClient, ptr, 0, NULL);				//���� �ֱٿ� �߰��� Ŭ���̾�Ʈ�� ����� ���ڷ� �������Լ��� ȣ���մϴ�
		if (hThread == NULL)
		{
			RemoveClient(ptr);																//���� ������ �߻��� �ش� Ŭ���̾�Ʈ�� �����ϰ�
			continue;																		//�ٽ� ���ο� Ŭ���̾�Ʈ�� �޽��ϴ�
		}
		else
		{
			CloseHandle(hThread);															//������ �߻����� ������� ������ ���� �������Լ������� ������ �����Ͽ� ���ο��� �ش� �������Լ��� �����Ҽ� �����մϴ�
		}
	}																						//������ ���������� �������� �ʴ��� �ݺ����� ��� ���⶧���� �ڵ������ ������ �������� �ʽ��ϴ�

	closesocket(listen_sock);																//���� �������� �ݽ��ϴ�							

	WSACleanup();																			//��� �����忡�� �����۾��� �����մϴ�
	return 0;
}

bool StringSend(char * _str, _ClientInfo* _player, PROTOCOL _protocol)	
			//���� : Ŭ���̾�Ʈ���� ���� ���ڿ�,Ŭ���̾�Ʈ ������ ���� _ClientInfo ����ü ������,� ������ ��Ŷ���� ������ ��������
{
	int str_size = strlen(_str);											//������ ���ڿ��� ���̸� �����մϴ�
	char * ptr = _player->packetbuf;										//������ Ŭ���̾�Ʈ�� ���ۿ� �����͸� �� �غ� �մϴ� 
	int size = sizeof(_protocol)+sizeof(str_size)+str_size;					//��Ŷ ������ ��üũ�⸦ ����մϴ�
			//���������� ũ�� + ���ڿ��� ���̸� ���� int�� ������ ũ�� + ���ڿ� ����

	switch (_protocol)														//�������ݷ� ����ġ���� �����Ͽ�
	{
	case QUESTION:															//QUESTION�� ���
		size += sizeof(_player->info->Chip) + sizeof(_player->info->BattingChip) + sizeof(_player->partner->info->Chip);		
		break;						//������ + �÷��̾��� Ĩ�� ������ int�� ������ ũ�� + �÷��̾ ������ Ĩ�� ������ int�� ������ ũ�� + �÷��̾��� ��Ʈ���� Ĩ�� ������ int�� ������ ũ��
	
	case OPEN:																//OPEN��
	case PARTNERDIE:														//DECKMAKING��
	case SPECIALDIE:														//SPECIALDIE�� ���
		size += sizeof(_player->info->Chip);								
		break;						//������ + �÷��̾��� Ĩ�� ������ int�� ������ ũ��
	}

	memcpy(ptr, &size, sizeof(size));										//�������� ��ġ�� ����� ����
	ptr = ptr + sizeof(size);												//������ �������� ũ�⸸ŭ �̵�

	memcpy(ptr, &_protocol, sizeof(_protocol));								//�������� ��ġ�� ���������� ����
	ptr = ptr + sizeof(_protocol);											//������ ���������� ũ�⸸ŭ �̵�

	memcpy(ptr, &str_size, sizeof(str_size));								//�������� ��ġ�� ���ڿ��� ũ�⸦ ����
	ptr = ptr + sizeof(str_size);											//������ ���ڿ��� ũ���� ũ�⸸ŭ �̵�

	memcpy(ptr, _str, str_size);											//�������� ��ġ�� ���ڿ��� ����
	ptr = ptr + str_size;													//������ ���ڿ��� ũ�⸸ŭ �̵�

	switch (_protocol)
	{
	case QUESTION:															//QUESTION�� ��� �߰��� �ش� case�� �ȿ� �ִ°͵� �����ϴ�
		memcpy(ptr, &_player->info->Chip, sizeof(_player->info->Chip));									//�������� ��ġ�� ������ Ĩ�� ����
		ptr = ptr + sizeof(_player->info->Chip);														//������ ������ Ĩ�� ũ�⸸ŭ �̵�

		memcpy(ptr, &_player->info->BattingChip, sizeof(_player->info->BattingChip));					//�������� ��ġ�� ������ ������ Ĩ�� ���� ����
		ptr = ptr + sizeof(_player->info->BattingChip);													//������ ������ ������ Ĩ�� ���� ũ�⸸ŭ �̵�

		memcpy(ptr, &_player->partner->info->BattingChip, sizeof(_player->partner->info->BattingChip));	//�������� ��ġ�� ������ ��Ʈ�ʰ� ������ Ĩ�� ���� ����
		ptr = ptr + sizeof(_player->partner->info->BattingChip);										//������ ������ ��Ʈ�ʰ� ������ Ĩ�� ���� ũ�⸸ŭ �̵�
		break;
									
	case OPEN:																//OPEN��
	case PARTNERDIE:														//DECKMAKING��
	case SPECIALDIE:														//SPECIALDIE�� ��� �߰��� �ش� case�� �ȿ� �ִ°͵� �����ϴ�
		memcpy(ptr, &_player->info->Chip, sizeof(_player->info->Chip));									//�������� ��ġ�� ������ Ĩ�� ����		
		ptr = ptr + sizeof(_player->info->Chip);														//������ ������ Ĩ�� ũ�⸸ŭ �̵�
		break;
	}

	if (send(_player->sock, _player->packetbuf, sizeof(size)+size, 0) == SOCKET_ERROR)	//������ ������� ��Ŷ�� �ش� Ŭ���̾�Ʈ���� �����ϴ�
	{
		char err_str[BUFSIZE];
		sprintf(err_str, "StringSend()%d", _protocol);									//������ ��Ÿ�� �������� ���������� �������ڷ� �к��մϴ�
		err_display(err_str);															//������ err_display�� ����ϰ�
		StopOver(_player->partner);														//StopOver�� send�� ������ ������ ��Ʈ�ʿ��� �¸� �޼����� �����ϴ�
		return false;																	//������ �Լ��� �����Ű������ false�� ��ȯ�մϴ�																
	}

	return true;																		//���� send���� ����ε� �۾��� �����Ͽ��ٸ� ���⿡�� �Լ��� �����ϴ�
}

void StopOver(_ClientInfo* _player)
			//���� : �¸� ó���� Ŭ���̾�Ʈ ������ ���� _ClientInfo ����ü ������
{
	char str[] = "��밡 ������ �������ϴ� ����� �¸��Դϴ�.\n";		//Ŭ���̾�Ʈ���� ���� ���ڿ�
	int str_size = strlen(str);											//���ڿ��� ũ��

	char * ptr = _player->packetbuf;									//������ Ŭ���̾�Ʈ�� ���ۿ� �����͸� �� �غ� �մϴ� 
	PROTOCOL protocol = COERCION;										//�������� ����
	int size = sizeof(protocol) + sizeof(str_size) + str_size;			//��Ŷ ������ ��üũ�⸦ ����մϴ�
			//���������� ũ�� + ���ڿ��� ũ�⸦ ������ int�� ������ ũ�� + ���ڿ��� ũ��
		
	memcpy(ptr, &size, sizeof(size));									//�������� ��ġ�� ����� ����
	ptr = ptr + sizeof(size);											//������ �������� ũ�⸸ŭ �̵�

	memcpy(ptr, &protocol, sizeof(protocol));							//�������� ��ġ�� ���������� ����
	ptr = ptr + sizeof(protocol);										//������ ���������� ũ�⸸ŭ �̵�

	memcpy(ptr, &str_size, sizeof(str_size));							//�������� ��ġ�� ���ڿ��� ũ�⸦ ����
	ptr = ptr + sizeof(str_size);										//������ ���ڿ��� ũ���� ũ�⸸ŭ �̵�

	memcpy(ptr, str, str_size);											//�������� ��ġ�� ���ڿ��� ����
	ptr = ptr + str_size;												//������ ���ڿ��� ũ�⸸ŭ �̵�

	send(_player->sock, _player->packetbuf, sizeof(size) + size, 0);	//������ ������� ��Ŷ�� �ش� Ŭ���̾�Ʈ���� �����ϴ�

	if (_player->dealer != nullptr)										//���� �ش� Ŭ���̾�Ʈ�� ��� ������ �������� �ʾҴٸ�
	{
		delete _player->dealer;											//������
		_player->dealer = nullptr;										//nullptr�� �ʱ�ȭ�մϴ�
	}

	RemoveClient(_player->partner);										//�� �÷��̾��� ��Ʈ�ʸ� �����ϰ�
	RemoveClient(_player);												//�� Ŭ���̾�Ʈ�� �����մϴ�								
}

int BattingStart(_ClientInfo* _player)									
			//���� : �޴� ���ñ��� ���� Ŭ���̾�Ʈ ������ ���� _ClientInfo ����ü ������
{
	int size = 0;														//��Ŷ�� ũ�⸦ ������ ����
	PROTOCOL protocol = NODATA;											//��Ŷ�� ������ ������ ����
	int retval = 0;														//recvn�� ���ϰ��� ������ ����
	char * ptr = nullptr;												//���ۿ� �������� ������ �о�� ������ ����

	retval = recvn(_player->sock, (char*)&size, sizeof(size), 0);		//recvn ���� ��Ŷ�� ��üũ�⸦ �о�ɴϴ�
	if (retval == SOCKET_ERROR || retval == 0)
	{
		err_display("recv error()1");
		return ERROR;
	}

	retval = recvn(_player->sock, _player->packetbuf, size, 0);			//recvn ���� �о�� ��üũ�⸸ŭ ��Ŷ�� �о�ɴϴ�
	if (retval == SOCKET_ERROR || retval == 0)
	{
		err_display("recv error()2");
		return ERROR;
	}

	ptr = _player->packetbuf;											//�����͸� ���ۿ� �������ְ�
	memcpy(&protocol, ptr, sizeof(protocol));							//protocol�� �����Ͱ� ����Ű�� ���� ����
	ptr = ptr + sizeof(protocol);										//������ protocol�� ũ�⸸ŭ �̵�

	if (protocol == ANSWER)												//���� ���������� ANSWER �ϰ�쿡�� �Ʒ� ������ ����
	{
		bool isWantDie;													//Ŭ���̾�Ʈ�� ���� �����ߴ��� ������ ����
		bool isWantOpen;												//Ŭ���̾�Ʈ�� ������ �����ߴ��� ������ ����
		int additionChip;												//Ŭ���̾�Ʈ�� ������ ���������� �߰����� �󸶳� �´��� ������ ����

		memcpy(&isWantDie, ptr, sizeof(isWantDie));						//isWantDie�� �����Ͱ� ����Ű�� ���� ����
		ptr = ptr + sizeof(isWantDie);									//������ isWantDie�� ũ�⸸ŭ �̵�

		if (!isWantDie)													//���� Ŭ���̾�Ʈ�� ù��° �������� NO�� ����������� �Ʒ� ������ ����
		{
			if (_player->partner->info->BattingChip > _player->info->BattingChip)				//���� Ŭ���̾�Ʈ�� ������ Ĩ���� Ŭ���̾�Ʈ�� ��Ʈ�ʰ� ������ Ĩ�� �������
			{
				int gap = _player->partner->info->BattingChip - _player->info->BattingChip;		//gap �̶�� ������ �ش� ���� ����

				if (_player->info->Chip - gap < 0)												//���� ���� Ĩ�� - gap �� 0���� �������
				{
					gap = _player->info->Chip;													//gap�� �ش� Ŭ���̾�Ʈ�� ���� Ĩ�� �ִ�ġ�� ����
				}

				_player->info->Chip -= gap;														//Ŭ���̾�Ʈ�� ���� Ĩ���� gap ��ŭ ����
				_player->info->BattingChip += gap;												//Ŭ���̾�Ʈ�� ������ Ĩ�� gap ��ŭ �ջ�
				_player->dealer->BattingChipSum += gap;											//Ŭ���̾�Ʈ�� �÷������� ������ ������ �������ִ� ��ü ���� Ĩ�� ���� gap ��ŭ �ջ�
			}

			/*
			if (_player->partner->info->BattingChip > _player->info->BattingChip) �� ���� ����
			�� �÷��̾�� �����ư��鼭 �޴��� ��½�ų���̰� �̰������� ���� ������ �÷��̾��
			���� �޴� ���ñ��� ���� �÷��̾��� ���̿��� ���ñ��� ���̰� �߻��մϴ�
			�� �÷��̾��� ���ñ��� �޴� ���ñ��� ���� �÷��̾ DIE�� �������� ������� ���ƾ��ϹǷ�
			���� ������ �����մϴ�

			���� �÷��̾ ���� Ĩ�� ������ �� ���̰� Ŭ��� ������ �ϴ°ɷ� �� ��Ȳ�� �ѱ�°��� �����մϴ�
			*/

			memcpy(&isWantOpen, ptr, sizeof(isWantOpen));				//isWantOpen �� �����Ͱ� ����Ű�� ���� ����
			ptr = ptr + sizeof(isWantOpen);								//������ isWantOpen�� ũ�⸸ŭ �̵�

			if (!isWantOpen)											//���� Ŭ���̾�Ʈ�� �ι�° �������� NO�� ����������� �Ʒ� ������ ����
			{
				memcpy(&additionChip, ptr, sizeof(additionChip));		//additionChip �� �����Ͱ� ����Ű�� ���� ����
				ptr = ptr + sizeof(additionChip);						//������ additionChip�� ũ�⸸ŭ �̵�

				_player->info->Chip -= additionChip;					//Ŭ���̾�Ʈ�� ���� Ĩ���� additionChip ��ŭ ����
				_player->info->BattingChip += additionChip;				//Ŭ���̾�Ʈ�� ������ Ĩ�� additionChip ��ŭ �ջ�
				_player->dealer->BattingChipSum += additionChip;		//Ŭ���̾�Ʈ�� �÷������� ������ ������ �������ִ� ��ü ���� Ĩ�� ���� additionChip ��ŭ �ջ�

				return BETTED;											//�ܺο� �ش� Ŭ���̾�Ʈ�� ������ �����ߴٰ� �˸�
			}

			return OPENED;												//�ܺο� �ش� Ŭ���̾�Ʈ�� ������ �����ߴٰ� �˸�
		}

		else
		{
			return DEAD;												//�ܺο� �ش� Ŭ���̾�Ʈ�� ���̸� �����ߴٰ� �˸�
		}

		/*
		�� �Լ������� ����
		�� �Լ��� �ι��� recvn�� �ϱ����� �Լ��Դϴ�
		ù��° recvn�� ��ü ����� �˾ƿ��� ��������
		�ι�° recvn�� bool���� 2���� ����Ͽ� �� 3������ �ϳ��� ��Ŷ�� �����ɴϴ�.

		1. Ŭ���̾�Ʈ�� ���̸� �����Ұ��
		��Ŷ�� ������ "��Ŷ�� ������ | �������� | DIE�� �����ߴ��� �ʾҴ��� ������ isWantDie ����" 
		���� 3���� ���빰�� ���������
		�̰�� isWantDie �� true�� ����Ű�� �ְ� �˴ϴ�

		2. Ŭ���̾�Ʈ�� ������ �����Ұ��
		��Ŷ�� ������ "��Ŷ�� ������ | �������� | isWantDie | OPEN�� �����ߴ��� �ʾҴ��� ������ isWantOpen"
		���� 4���� ���빰�� ���������
		�̰�� isWantDie �� false, isWantOpen �� true�� ����Ű�� �˴ϴ�

		3. Ŭ���̾�Ʈ�� ������ �����Ұ��
		��Ŷ�� ������ "��Ŷ�� ������ | �������� | isWantDie | isWantOpen | �߰� ���ñ�"
		���� 5���� ���빰�� ���������
		�̰�� isWantDie �� false, isWantOpen �� false�� ����Ű�� �˴ϴ�
		*/

	}

	return ERROR;														//�� �κп��� �����°��� �����Դϴ�
}

DWORD CALLBACK ProcessClient(LPVOID arg)								//������ �Լ�
{	
	int battingResult = QUESTIONSEND;									//BattingStart �Լ��� ����� ������ ����
	int mainPlayer = 0;													//�޴������ �ؼ� �޴������� ���ñ��� ���� ����
	bool newCardFlag = true;
	_ClientInfo* player[2];												//�ΰ��� Ŭ���̾�Ʈ ������ �迭
	player[0] = (_ClientInfo*)arg;										//0���� ���ڷο� arg
	player[1] = player[0]->partner;										//1���� 0���� ��Ʈ��

	Dealer_info * gameDealer = new Dealer_info;							//������ ���۵Ǳ��� ���ο� ������ ����
	DeckMaking(gameDealer);												//ī�带 ����

	player[0]->dealer = gameDealer;										//0�� �÷��̾��� ��� ������ ������ ���� ������ ����
	player[1]->dealer = gameDealer;										//1�� �÷��̾��� ��� ������ ������ ���� ������ ����

	gameDealer->BattingChipSum = 0;										//���� ����Ĩ ���� 0���� �ʱ�ȭ			

	while (1)
	{
		int size = 0;													//��Ŷ�� ���� ����� ������ ����
		PROTOCOL protocol = NODATA;										//��Ŷ�� ������ ���� �������� ����
		int retval = 0;													//�Լ��� ���ϰ� �����
		char * ptr = nullptr;											//��Ŷ�� ���� ������
		char str[BUFSIZE] = "";											//���ڿ��� ������ ĳ���͹迭
		int str_size = 0;												//���ڿ��� ũ�⸦ ������ ����

		if (gameDealer->DeckCount == 0)									//���� ī�带 ���� ���������� �Ʒ� ������ �����մϴ�
		{
			sprintf(str, "\n���� : 20���� ī�带 ��� ����Ͽ����ϴ�. \n���ο� ���� ����ڽ��ϴ�.\n");
			//�̿� ���� ���ڿ��� str�� �����մϴ�
			for (int i = 0; i < 2; i++)
			{
				if (!StringSend(str, player[i], DECKMAKING))
				{
					return ERROR;
				}					//���� ���ڿ��� Ŭ���̾�Ʈ���� �����մϴ�
			}

			DeckMaking(gameDealer);										//���ο� ���� ����ϴ�
		}

		if (newCardFlag)
		{
			for (int i = 0; i < 2; i++)
			{
				player[i]->info->Chip -= 1;												//�⺻������ ���� �� �÷��̾��� Ĩ�� 1�� �����ϴ�
				player[i]->info->BattingChip += 1;										//�⺻������ ���� �� �÷��̾ ������ Ĩ�� 1�� �ø��ϴ�
				gameDealer->BattingChipSum += 1;										//�⺻������ �ݾ��� ���� ������ �����صӴϴ�
				player[i]->info->Card = gameDealer->Deck[gameDealer->DeckCount - 1];	//�� �÷��̾�� ���ο� ī�带 �޽��ϴ�
				gameDealer->DeckCount--;												//������ �����ϰ��մ� ī���� ���� ���Դϴ�
			}

			newCardFlag = false;
		}

		for (int i = 0; i < 2; i++)														//2�� ������ �ݺ����� �غ��ϰ�
		{
			sprintf(str, "\n����� ī�� : %d \n���� ����� Ĩ : %d \n���� ����� Ĩ : %d \n���� ����� ������ Ĩ : %d \n���� ��밡 ������ Ĩ : %d \n���� ���õ� Ĩ�� �� : %d \n",
				player[i]->partner->info->Card, player[i]->info->Chip, player[i]->partner->info->Chip, player[i]->info->BattingChip, player[i]->partner->info->BattingChip, gameDealer->BattingChipSum);
			str_size = strlen(str);														//���� ���� ���ڿ��� str�� �����մϴ�

			ptr = player[i]->packetbuf;													//�����ʹ� i��° �÷��̾��� ����
			protocol = INTRO;															//���������� ��Ʈ�η� �����մϴ�.
			size = sizeof(protocol) + sizeof(str_size) + str_size + sizeof(player[i]->info->Chip) + sizeof(player[i]->info->BattingChip) + sizeof(player[i]->partner->info->BattingChip);
				//������� ���������� ũ�� + ���ڿ��� ũ�⸦ ������ ������ ũ�� + ���ڿ��� ũ�� + i ��° �÷��̾��� Ĩ���� ������ ������ ũ�� 
				//+ i ��° �÷��̾ ������ Ĩ�� ������ ������ ũ�� + i ��° �÷��̾��� ��Ʈ�ʰ� ������ Ĩ�� ������ ������ ũ�� �Դϴ�

			memcpy(ptr, &size, sizeof(size));																		//�������� ��ġ�� ����� ����
			ptr = ptr + sizeof(size);																				//������ �������� ũ�⸸ŭ �̵�

			memcpy(ptr, &protocol, sizeof(protocol));																//�������� ��ġ�� ���������� ����
			ptr = ptr + sizeof(protocol);																			//������ ���������� ũ�⸸ŭ �̵�

			memcpy(ptr, &str_size, sizeof(str_size));																//�������� ��ġ�� ���ڿ��� ũ�⸦ ����
			ptr = ptr + sizeof(str_size);																			//������ ���ڿ��� ũ���� ũ�⸸ŭ �̵�

			memcpy(ptr, str, str_size);																				//�������� ��ġ�� ���ڿ��� ����
			ptr = ptr + str_size;																					//������ ���ڿ��� ũ�⸸ŭ �̵�

			memcpy(ptr, &player[i]->info->Chip, sizeof(player[i]->info->Chip));										//�������� ��ġ�� �÷��̾��� Ĩ���� ����
			ptr = ptr + sizeof(player[i]->info->Chip);																//������ �÷��̾��� Ĩ���� ũ�⸸ŭ �̵�

			memcpy(ptr, &player[i]->info->Chip, sizeof(player[i]->info->BattingChip));								//�������� ��ġ�� �÷��̾ ������ Ĩ�� ����
			ptr = ptr + sizeof(player[i]->info->BattingChip);														//������ �÷��̾ ������ Ĩ�� ũ�⸸ŭ �̵�

			memcpy(ptr, &player[i]->partner->info->BattingChip, sizeof(player[i]->partner->info->BattingChip));		//�������� ��ġ�� �÷��̾��� ��Ʈ�ʰ� ������ Ĩ�� ����
			ptr = ptr + sizeof(player[i]->partner->info->BattingChip);												//������ �÷��̾��� ��Ʈ�ʰ� ������ Ĩ�� ũ�⸸ŭ �̵�

			if (send(player[i]->sock, player[i]->packetbuf, sizeof(size) + size, 0) == SOCKET_ERROR)				//������ ���� ��Ŷ�� send�մϴ�
			{
				err_display("IntroSend()");																				//������ �̷� ���ڿ��� �ٿ���̸�
				StopOver(player[i]->partner);																			//��Ʈ�ʿ��� �¸��޼����� �����ϴ�
				return STOP_OVER;																						//�Լ��� �����ŵ�ϴ�
			}
		}

		if (battingResult == QUESTIONSEND)																				//�̹��Ͽ� �޴� ���ñǸ� ���� �÷��̾�� �޴��� ����ϱ����� if���Դϴ�
		{
			sprintf(str, "��밡 ���ñ��� �������ֽ��ϴ�.\n");															//�̷��� ���ڿ���
			if (!StringSend(str, player[mainPlayer]->partner, WAIT))													//WAIT�������ݰ� �Բ� ���ñ��� ���� ������ ��Ʈ�ʿ��� ������
			{
				return STOP_OVER;
			}
			
			sprintf(str, "Die ? 1. yes, 2. no \nAnswer :");																//�̷��� ���ڿ���
			if (!StringSend(str, player[mainPlayer], QUESTION))															//QUESTION �������ݰ� �Բ� �޴����ñ��� ���� �������� �����ϴ� 
			{
				return STOP_OVER;
			}
			battingResult = BattingStart(player[mainPlayer]);															//�׸��� BattingStart �Լ��� �����Ͽ� ����� battingResult �� �����մϴ�
		}

		if (battingResult == DEAD)																							//���� BattingStart�� ����� DEAD �������
		{
			if (player[mainPlayer]->info->Card == 10)																		//���� �ش� �÷��̾ �������ִ� ī�尡 10�̿������
			{
				player[mainPlayer]->partner->info->Chip += 10;															
				player[mainPlayer]->info->Chip -= 10;																		//��Ʈ�ʿ��� 10���� Ĩ�� ���ѱ�ϴ�

				sprintf(str, "\n��밡 DIE�� �����߽��ϴ�. \n��밡 10�� ������ �й������Ƿ� 10���� Ĩ�� �޽��ϴ�.\n");		//�̷��� ���ڿ���
				if (!StringSend(str, player[mainPlayer]->partner, SPECIALDIE))												//SPECIALDIE �������ݰ� �Բ� �ش� �÷��̾��� ��Ʈ�ʿ��� ������
				{
					return STOP_OVER;
				}

				sprintf(str, "\n����� 10�� ������ �й��Ͽ����ϴ�. \n��뿡�� 10���� Ĩ�� �ݴϴ�.\n");						//�̷��� ���ڿ���
				if (!StringSend(str, player[mainPlayer], SPECIALDIE))														//SPECIALDIE �������ݰ� �Բ� �ش� �÷��̾�� �����ϴ�
				{
					return STOP_OVER;
				}
			}

			else																											//�׿��� ī�带 ������ �����Ұ��
			{
				sprintf(str, "\n��밡 DIE�� �����߽��ϴ�. \n����� ī�� ������ : %d\n", player[mainPlayer]->info->Card);	//�� Ŭ���̾�Ʈ ��� �ڽ��� ī�带 �������� �ʰ� �����ϴ�
				if (!StringSend(str, player[mainPlayer]->partner, PARTNERDIE))												//���� ���ڿ��� PARTNERDIE ���������� �ش� �÷��̾��� ��Ʈ�ʿ��� �����ϴ�
				{
					return STOP_OVER;
				}
			}

			player[mainPlayer]->info->BattingChip = 0;																	//�޴� ���ñ��� ���� �÷��̾ �����ߴ� Ĩ�� 0���� �����
			player[mainPlayer]->partner->info->BattingChip = 0;															//�� ��Ʈ�ʰ� �����ߴ� Ĩ�� 0���� ����ϴ�
			player[mainPlayer]->partner->info->Chip += gameDealer->BattingChipSum;										//���� ��Ʈ�ʴ� ������ �����ϰ��ִ� ����Ĩ�� �հ踦 �ڽ��� �������ִ� ��üĨ�� �ջ��մϴ�
			gameDealer->BattingChipSum = 0;																				//���������� ������ �������ִ� �� �÷��̾��� ����Ĩ�� �հ踦 0���θ���ϴ�
			newCardFlag = true;
		}

		mainPlayer = (mainPlayer == 1) ? 0 : 1;																			//�޴� ���ñ��� ���� �÷��̾ �ٲߴϴ�

		if (battingResult == OPENED)																					//���� BattingStart�� ����� OPENED �������
		{
			for (int i = 0; i < 2; i++)																					//���÷��̾�� ������ֱ����� 2���� �ݺ����� �غ��ϰ�
			{
				//�¸�
				if (player[i]->info->Card > player[i]->partner->info->Card)													//���� i��° �÷��̾ �������ִ� ī�尡 ��Ʈ���� ī�庸�� ũ�ٸ�
				{
					player[i]->info->BattingChip = 0;																		//i��° �÷��̾ ������ Ĩ�� 0���� �ʱ�ȭ���ݴϴ�
					player[i]->info->Chip += gameDealer->BattingChipSum;													//�ش� �÷��̾��� Ĩ�� ������ �������ִ� ����Ĩ�� �հ踦 �ջ��մϴ�
					gameDealer->BattingChipSum = 0;																			//���� ������ �������ִ� ����Ĩ�� �հ踦 0���� �ʱ�ȭ�մϴ�
					sprintf(str, "\n������ ���õǾ����ϴ�. \n����� ī�� : %d, ����� ī�� : %d \n�¸��Դϴ�.\n���� ����� Ĩ : %d\n", player[i]->info->Card, player[i]->partner->info->Card, player[i]->info->Chip);
					mainPlayer = i;																							//���ڿ��� ���� ���ڿ��� �����ϰ� �������� �޴� ���ñ��� ���ڿ��� �ݴϴ�
				}

				//�й�
				else if (player[i]->info->Card < player[i]->partner->info->Card)											//���� i��° �÷��̾ �������ִ� ī�尡 ��Ʈ���� ī�庸�� �۴ٸ�
				{
					player[i]->info->BattingChip = 0;																		//i��° �÷��̾ ������ Ĩ�� 0���� �ʱ�ȭ���ݴϴ�
					sprintf(str, "\n������ ���õǾ����ϴ�. \n����� ī�� : %d, ����� ī�� : %d \n�й��Դϴ�.\n���� ����� Ĩ : %d\n", player[i]->info->Card, player[i]->partner->info->Card, player[i]->info->Chip);
				}																											//���� ���ڿ��� �����մϴ�	

				//���º�
				else																										//���� i��° �÷��̾ �������ִ� ī�尡 ��Ʈ���� ī���ϰ� �Ȱ��ٸ�
				{
					sprintf(str, "\n������ ���õǾ����ϴ�. \n����� ī�� : %d, ����� ī�� : %d \n���º��Դϴ�.\n���� ����� Ĩ : %d\n���õ� Ĩ�� �����ǿ� ���˴ϴ�.\n", player[i]->info->Card, player[i]->partner->info->Card, player[i]->info->Chip);
				}																											//���� ���ڿ��� �����մϴ�. �׸��� ������ ������ ����Ĩ�� �հ�� 0���� �ʱ�ȭ���� ������ �����ǿ� ���˴ϴ�

				if (!StringSend(str, player[i], OPEN))																		//������ ������ ���ڿ��� OPEN �������ݰ� �Բ� i��° �÷��̾�� �����ϴ�
				{
					return STOP_OVER;
				}
				newCardFlag = true;																							//������ �Ͽ����� ���ο� ī�带 �ֱ����� �÷��׸� ����ϴ�
			}
		}

		battingResult = QUESTIONSEND;																					//�� �÷��̾ �޴����� ��Ѱ��� �����Ͽ��� �Ǵٽ� �޴� ���ñ��� ���� �÷��̾�� �޴��� ��½�������ϹǷ� QUESTIONSEND�� battingResult �� �ʱ�ȭ���ݴϴ�

		for (int i = 0; i < 2; i++)																						//2�� �˻��ϱ����� 2���� �������� �غ����ְ�
		{
			if (player[i]->info->Chip <= 0 && player[i]->info->BattingChip == 0 && gameDealer->BattingChipSum == 0)		//i ��° �÷��̾��� Ĩ�� 0�����̸� ������ Ĩ�� 0���̸� ������ ������ ����Ĩ�� �հ谡 0���϶�
			{
				sprintf(str, "����� Ĩ�� ���� �����߽��ϴ�. �й��Ͽ����ϴ�.\n");										//�ش� �÷��̾�� �й��Ͽ����Ƿ� �̷��� ���ڿ���
				if (!StringSend(str, player[i], GAMESET))																//GAMESET �������ݰ� �Բ� �����ݴϴ�
				{
					return STOP_OVER;
				}

				sprintf(str, "��밡 Ĩ�� ���� �����߽��ϴ�. �¸��Ͽ����ϴ�.\n");										//�׸��� �� �÷��̾��� ��Ʈ�ʿ��� �̷��� ���ڿ���
				if (!StringSend(str, player[i]->partner, GAMESET))														//GAMESET �������ݰ� �Բ� �����ݴϴ�
				{
					return STOP_OVER;
				}

				if (gameDealer != nullptr)																				//������ ���� ������ �ȵǾ��ٸ�
				{
					delete gameDealer;																					//������ �����ϰ�
					gameDealer = nullptr;																				//nullptr���� �������ݴϴ�
				}

				RemoveClient(player[i]->partner);																		//���� ��Ʈ�ʸ� �켱������ �������ְ�
				RemoveClient(player[i]);																				//i ��° �÷��̾ �����մϴ�

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

// ���� �Լ� ���� ���
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
					//���� : ����,���� ����ü
{
	_ClientInfo* ptr = new _ClientInfo;				//���ο� Ŭ���̾�Ʈ ����ü �����Ҵ�
	ZeroMemory(ptr, sizeof(_ClientInfo));			//�ش� �����Ҵ� ����ü �ʱ�ȭ
	ptr->sock = sock;								//���� ����
	memcpy(&(ptr->clientaddr), &clientaddr, sizeof(clientaddr));
													//���� ����ü ����
	ptr->info = new User_Info;						//Ŭ���̾�Ʈ �� �������� ����ü �����Ҵ�
	ptr->info->BattingChip = 0;						//������ ������ Ĩ�� �� 0���� �ʱ�ȭ
	ptr->info->Card = 0;							//������ ����ִ� ī�� 0���� �ʱ�ȭ
	ptr->info->Chip = 30;							//������ �������ִ� Ĩ 30���� �ʱ�ȭ

	ClientInfo[count] = ptr;						//ClientInfo �迭�� count ������ �ش� Ŭ���̾�Ʈ ���� ����
	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));				//�������� ���
	count++;										//count ����

	return ptr;
}

void RemoveClient(_ClientInfo* ptr)
				//���� : ������ _ClientInfo ����ü�� ������ 
{
	closesocket(ptr->sock);								//�ش� Ŭ���̾�Ʈ�� ������ �ݽ��ϴ�

	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(ptr->clientaddr.sin_addr),
		ntohs(ptr->clientaddr.sin_port));				//�������� ���

	for (int i = 0; i < count; i++)						//count��ŭ �� �ݺ�������
	{
		if (ClientInfo[i] == ptr)						//ClientInfo�� �ش� Ŭ���̾�Ʈ�� �ִ��� Ȯ��
		{
			if (ptr->partner != nullptr)				//������ Ŭ���̾�Ʈ�� ��Ʈ�ʰ� �������Ͱ� �ƴѰ��
			{
				ptr->partner->partner = nullptr;		//������ Ŭ���̾�Ʈ�� ��Ʈ�ʰ� �ڽ��� ����Ű�°� �������͸� ����Ű�� �ٲ���
			}

			delete ptr->info;							//������ Ŭ���̾�Ʈ�� ���������� ����
			delete ptr;									//�ش� Ŭ���̾�Ʈ ����

			int j;
			for (j = i; j < count - 1; j++)				//count - 1 ��ŭ ������
			{
				ClientInfo[j] = ClientInfo[j + 1];		//������ Ŭ���̾�Ʈ�� ��ġ�������� �ڿ��ִ� Ŭ���̾�Ʈ���� ������
			}
			ClientInfo[j] = nullptr;					//������ Ŭ���̾�Ʈ nullptr
			break;
		}
	}

	count--;											//���� �Ϸ��� count 1����

}

bool MatchPartner(_ClientInfo* _ptr)
			//���� : ��Ʈ�ʸ� ã�ƾ��ϴ� _ClientInfo ����ü�� ������
{
	for (int i = 0; i < count; i++)											//count ��ŭ ���� �ݺ���
	{
		if (_ptr != ClientInfo[i] && ClientInfo[i]->partner == nullptr)		//���� ��Ʈ�ʰ� ���� Ŭ���̾�Ʈ�� �ְ� �װ� �ڽ��� �ƴҰ�쿡��
		{
			ClientInfo[i]->partner = _ptr;
			_ptr->partner = ClientInfo[i];									//���� ��Ʈ�� ����
			return true;													//���� ��ȯ
		}
	}

	return false;															//������ ��ȯ���� ���Ұ�� ���� ��ȯ
}

void DeckMaking(Dealer_info * ptr)
				//���� : ���ο� ���� �������ϴ� Dealer_info ������
{
	srand((unsigned)time(NULL));					

	for (int j = 0; j < 10; j++)					//10�� ���� �ݺ����� �غ�
	{
		ptr->Deck[j] = j + 1;						//j��°�� j+1�� �ְ�
		ptr->Deck[j + 10] = j + 1;					//���������� j+10�� ��ġ���� j+1�� �ֽ��ϴ�.
	}												//ī��� 1~10�� ���� 2�徿 �ֱ⶧���Դϴ�.

	int ShuffleCount = (rand() % 20) + 30;			//�ּ� 30 ~ �ִ� 49 �� ���� �� ����

	for (int i = 0; i < ShuffleCount; i++)			//ShuffleCount ��ŭ �ݺ�
	{
		int randA;									//������ ����� 1
		int randB;									//������ ����� 2
		int temp;									//���ҽ� �ӽ������

		while (1)
		{
			randA = (rand() % 20);					//�ּ� 0 ~ �ִ� 19
			randB = (rand() % 20);					//�� �������� �ΰ� ����

			if (randA != randB)						//���� ���� ���´ٸ� �ٽû���
			{
				break;								//�ƴ϶�� �ݺ��� Ż��
			}
		}

		temp = ptr->Deck[randA];					//randA ��° ī���
		ptr->Deck[randA] = ptr->Deck[randB];		//randB ��° ī����
		ptr->Deck[randB] = temp;					//��ġ�� �ٲߴϴ�
	}

	ptr->DeckCount = FULLDECK;						//ī�尡 ����á�ٴ°� �˸������� FULLDECK ���� �ʱ�ȭ
}