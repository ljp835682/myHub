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
					
struct ClaData													//Ŭ���̾�Ʈ���� �ʿ��� ������ ������ ����ü
{
	int Chip;													//���� ���� Ĩ��
	int BattingChip;											//���� �ڽ��� ������ Ĩ��
	int PartnerBattingChip;										//���� ��Ʈ�ʰ� ������ Ĩ��
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

  bool isBaseBet = true;										//�޼��� ������� ������ ���� ����

  printf("\n\n��� ��ٷ��ּ���\n\n");							

  while (1)
  {
	  if (isBaseBet)
	  {
		  printf("1���� Ĩ�� �⺻�����մϴ�\n\n");				
		  isBaseBet = false;									//�ٽ� ����Ҽ����� false�� �ٲߴϴ�
																//PARTNERDIE Ȥ�� OPEN Ȥ�� SPECIALDIE �� ������ ���������� ������ �ٽ� true�� ���մϴ�
	  }

	  char buf[BUFSIZE];										//Ŭ���̾�Ʈ�� ����
	  int size = 0;												//��Ŷ�� ���� ������
	  char str[BUFSIZE] = "";									//���ڿ��� ������ �迭
	  int str_size = 0;											//���ڿ��� ���̸� ������ ����
	  PROTOCOL protocol = NODATA;								//��Ŷ�� ������ ���� ��������
	  bool ExitFlag = false;									//���α׷� ���� �÷���

	  result = recvn(hSocket, (char*)&size, sizeof(size), 0);	//Ŭ���̾�Ʈ�� �⺻������ �������� ���𰡸� ������������ �����մϴ�
	  if (result == SOCKET_ERROR || result == 0)				//�ϴ� ���𰡸� ������ �ְԵȴٸ� �� ��Ŷ�� ��ü ũ�⸦ ���� �޾ƺ��ϴ�
	  {
		  err_display("recv error()1");
		  return -1;
	  }

	  result = recvn(hSocket, buf, size, 0);					//�׸��� �� ��Ŷ�� ���� ������
	  if (result == SOCKET_ERROR || result == 0)
	  {
		  err_display("recv error()2");
		  return -1;
	  }

	  char* ptr = buf;											//��Ŷ �б�� �����͸� ������ְ�
	  memcpy(&protocol, ptr, sizeof(protocol));					//�켱 ���������� ���� �о�ϴ�
	  ptr = ptr + sizeof(protocol);								//���� ���������� ������ŭ �����͸� �̵���ŵ�ϴ�

	  bool isWantDie = true;										//Ŭ���̾�Ʈ�� ���̸� �����ߴ��� ���ߴ��� ������ ����
	  bool isWantOpen = true;										//Ŭ���̾�Ʈ�� ������ �����ߴ��� ���ߴ��� ������ ����
	  int additionChip = 0;											//Ŭ���̾�尡 �󸶳� �� �߰����� �´��� ������ ����

	  switch (protocol)											//�������ݷ� ����ġ���� �����Ų��
	  {
	  case INTRO:												//INTRO�� ���
		  {
			  //------------------------------------------------------------------------------
			  //INTRO ����

			  memcpy(&str_size, ptr, sizeof(str_size));										//str_size �� ���ڿ��� ���̸� ����
			  ptr = ptr + sizeof(str_size);													//str_size �� ũ�⸸ŭ ������ �̵�

			  memcpy(str, ptr, str_size);													//str �� ���ڿ� ����
			  ptr = ptr + str_size;															//str_size ��ŭ ������ �̵�

			  memcpy(&info->Chip, ptr, sizeof(info->Chip));									//info->Chip �� ������ Ĩ�� ���� ����
			  ptr = ptr + sizeof(info->Chip);												//info->Chip�� ũ�⸸ŭ ������ �̵�

			  memcpy(&info->BattingChip, ptr, sizeof(info->BattingChip));					//info->BattingChip �� ������ ������ Ĩ�� ���� ����
			  ptr = ptr + sizeof(info->BattingChip);										//info->BattingChip�� ũ�⸸ŭ ������ �̵�

			  memcpy(&info->PartnerBattingChip, ptr, sizeof(info->PartnerBattingChip));		//info->PartnerBattingChip �� ��Ʈ�ʰ� ������ Ĩ�� ���� ����
			  ptr = ptr + sizeof(info->PartnerBattingChip);									//info->PartnerBattingChip�� ũ�⸸ŭ ������ �̵�

																							//���⼭ info->Chip, info->BattingChip, info->PartnerBattingChip �� ���� ������
																							//�ʱ�ȭ�Դϴ�

			  str[str_size] = '\0';															//���ڿ� ���� �ι��� ������
			  printf("%s", str);															//���
			  break;																		//switch case�� ����
		  }
      case QUESTION:												//QUESTION�� ���
		  {
			  //------------------------------------------------------------------------------
			  //QUESTION ����

			  memcpy(&str_size, ptr, sizeof(str_size));										//str_size �� ���ڿ��� ���̸� ����
			  ptr = ptr + sizeof(str_size);													//str_size �� ũ�⸸ŭ ������ �̵�

			  memcpy(str, ptr, str_size);													//str �� ���ڿ� ����
			  ptr = ptr + str_size;															//str_size ��ŭ ������ �̵�

			  memcpy(&info->Chip, ptr, sizeof(info->Chip));									//info->Chip �� ������ Ĩ�� ���� ����
			  ptr = ptr + sizeof(info->Chip);												//info->Chip�� ũ�⸸ŭ ������ �̵�

			  memcpy(&info->BattingChip, ptr, sizeof(info->BattingChip));					//info->BattingChip �� ������ ������ Ĩ�� ���� ����
			  ptr = ptr + sizeof(info->BattingChip);										//info->BattingChip�� ũ�⸸ŭ ������ �̵�

			  memcpy(&info->PartnerBattingChip, ptr, sizeof(info->PartnerBattingChip));		//info->PartnerBattingChip �� ��Ʈ�ʰ� ������ Ĩ�� ���� ����
			  ptr = ptr + sizeof(info->PartnerBattingChip);									//info->PartnerBattingChip�� ũ�⸸ŭ ������ �̵�
																							
																							//���⼭ info->Chip, info->BattingChip, info->PartnerBattingChip �� ���� ������
																							//���������� ��ӵǴ� ����, ����, �������� �� ��ġ�� ��� ����������
																							//�̽������� ������ �ʴ´ٸ� Ŭ���̾�Ʈ������ �̸� ������ ����� �����ϴ�

			  str[str_size] = '\0';															//���� ���ڿ� �ڿ� �ι��ڸ� �����ϴ�

			  //------------------------------------------------------------------------------
			  //QUESTION ���

			  while (1)
			  {
				  int c;
				  printf("%s", str);														//������ ���� ���ڿ��� ����մϴ�
				  scanf("%d", &c);															//�׸��� �Է��� �ް�
				  if (c == 1 || c == 2)
				  {
					  isWantDie = (c == 2) ? false : true;									//���ÿ����� isWantDie�� ���� ������ŵ�ϴ�
					  break;
				  }
			  }
			  //------------------------------------------------------------------------------
			  //ANSWER ����

			  ptr = buf;																	//���ۿ� �����ְ� �����͸� �ʱ�ȭ���ְ�
			  protocol = ANSWER;															//���������� ANSWER �� �������ݴϴ�
			  size = sizeof(protocol) + sizeof(isWantDie);									//����� ���մϴ�
					//���������� ũ�� + ������ �������� ���� bool ����

			  //----------------------���̸� �����Ѵٸ� ���⼭ size ��ž----------------------

			  if (!isWantDie)																//���̸� �������� �������
			  {
				  if (info->PartnerBattingChip > info->BattingChip)							//Ĩ�� ������ ���̳��� �� ���̸� �Ųٴ� �κ��Դϴ�
				  {
					  int gap = info->PartnerBattingChip - info->BattingChip;					//gap�� ��Ʈ�ʰ� �� Ĩ�� ������ �ڽ��� �� Ĩ�� ������ ���� �����մϴ�

					  if (info->Chip - gap < 0)													//���� ���� Ĩ���� gap�� ������ 0���� �������
					  {
						  gap = info->Chip;														//gap�� ���� Ĩ�� ������ �Ȱ������ϴ�
					  }

					  if (gap != 0)																//gap�� 0�� �ƴҶ���
					  {
						  printf("\n���� : %d���� Ĩ�� ���̰� ���ϴ� �Ųٰڽ��ϴ�.\n", gap);	//�̷��� �޼����� ���ϴ�
					  }

					  info->Chip -= gap;														//����Ĩ���� gap�� �����ϰ�
					  info->BattingChip += gap;													//������Ĩ�� gap�� �ջ��մϴ�

					  printf("���� Ĩ : %d.\n", info->Chip);									//�Ի��ϰ� ����Ĩ�� �����ݴϴ�
				  }

				  if (info->Chip > 0)														 //���� �÷��̾��� ���� Ĩ�� 0�� ���϶�� �����ʰ� �����մϴ�
				  {
					  while (1)
					  {
						  int c;
						  printf("\n1. ���� 2. ����\n");									 //�������� �������� ���ºκ��Դϴ�
						  printf("Answer : ");
						  scanf("%d", &c);

						  if (c == 1 || c == 2)
						  {
							  isWantOpen = (c == 2) ? false : true;							 //������ ������ ���� ���� isWantOpen�� ���� �����˴ϴ�
							  break;
						  }
					  }
				  }

				  size += sizeof(isWantOpen);												 //�׸��� size�� isWantOpen�� ũ�⸦ ���մϴ�

				  //----------------------������ �����Ѵٸ� ���⼭ size ��ž----------------------

				  
				  if (!isWantOpen)															 //���� ������ ���� �ʴ°�� 
				  {
					  while (1)
					  {
						  printf("\n���� Ĩ : %d\n", info->Chip);
						  printf("���� Ĩ�� �� �����ϰڽ��ϱ�? : ");						 //������ �󸶳� ������ ����
						  scanf("%d", &additionChip);										 

						  //����Ĩ���ϸ� �Է°���
						  if (info->Chip >= additionChip && additionChip > 0)				 //��Ȯ�� ���� �Է��ߴٸ�
						  {
							  info->Chip - additionChip;
							  info->BattingChip += additionChip;
							  break;
						  }
					  }

					  size += sizeof(additionChip);											 //size�� additionChip�� ũ�⸦ ���մϴ�

					  //----------------------������ �����Ѵٸ� ���⼭ size ��ž----------------------
				  }
			  }

			  memcpy(ptr, &size, sizeof(size));											//�������� ��ġ�� ����� ����
			  ptr = ptr + sizeof(size);													//������ �������� ũ�⸸ŭ �̵�

			  memcpy(ptr, &protocol, sizeof(protocol));									//�������� ��ġ�� ���������� ����
			  ptr = ptr + sizeof(protocol);												//������ ���������� ũ�⸸ŭ �̵�

			  memcpy(ptr, &isWantDie, sizeof(isWantDie));								//�������� ��ġ�� isWantDie�� ����
			  ptr = ptr + sizeof(isWantDie);											//������ isWantDie�� ũ�⸸ŭ �̵�

			  //----------------------���̸� �����ߴٸ� ������� send()----------------------

			  if (!isWantDie)																//���� isWantDie �� false �ϰ��
			  {		
				  memcpy(ptr, &isWantOpen, sizeof(isWantOpen));								//�������� ��ġ�� isWantOpen�� ����
				  ptr = ptr + sizeof(isWantOpen);											//������ isWantOpen�� ũ�⸸ŭ �̵�

				  //----------------------���¸� �����ߴٸ� ������� send()----------------------

				  if (!isWantOpen)																//���� isWantOpen �� false �ϰ��
				  {
					  memcpy(ptr, &additionChip, sizeof(additionChip));							//�������� ��ġ�� �߰����ñ��� ����
					  ptr = ptr + sizeof(additionChip);											//������ �߰����ñ��� ũ�⸸ŭ �̵�

					  //----------------------���ø� �����ߴٸ� ������� send()----------------------
				  }
			  }

			  if (send(hSocket, buf, sizeof(size) + size, 0) == SOCKET_ERROR)			//������ ���� ��Ŷ�� �������� �۽��մϴ�
			  {
				  err_display("DieSend()");
				  return SERVERDROP;
			  }

			  break;
		  }
	  case WAIT:
	  case DECKMAKING:												//DECKMAKING�� ���
		  {
			  memcpy(&str_size, ptr, sizeof(str_size));										//str_size �� ���ڿ��� ���̸� ����
			  ptr = ptr + sizeof(str_size);													//str_size �� ũ�⸸ŭ ������ �̵�

			  memcpy(str, ptr, str_size);													//str �� ���ڿ� ����
			  ptr = ptr + str_size;															//str_size ��ŭ ������ �̵�

			  str[str_size] = '\0';															//���ڿ� ���� �ι���

			  printf("%s", str);															//���
			  break;
		  }

	  case OPEN:													//OPEN��
	  case PARTNERDIE:												//DECKMAKING��
	  case SPECIALDIE:												//SPECIALDIE�� ���
		  {
			  memcpy(&str_size, ptr, sizeof(str_size));										//str_size �� ���ڿ��� ���̸� ����
			  ptr = ptr + sizeof(str_size);													//str_size �� ũ�⸸ŭ ������ �̵�

			  memcpy(str, ptr, str_size);													//str �� ���ڿ� ����
			  ptr = ptr + str_size;															//str_size ��ŭ ������ �̵�

			  memcpy(&info->Chip, ptr, sizeof(info->Chip));									//info->Chip �� ���� ������ Ĩ���� ����
			  ptr = ptr + sizeof(info->Chip);												//info->Chip�� ũ�⸸ŭ ������ �̵�

			  info->BattingChip = 0;														//���� ������ Ĩ�� 0���� �ʱ�ȭ�ϰ�
			  info->PartnerBattingChip = 0;													//����� ������ Ĩ�� 0���� �ʱ�ȭ�մϴ�

			  str[str_size] = '\0';															//���ڿ� ���� �ι���
			  printf("%s", str);															//���

			  isBaseBet = true;																//�ٽ��ѹ� �⺻���� �޼����� �������� isBaseBet�� true�� �ٲߴϴ�
			  break;
		  }

	  case GAMESET:													//GAMESET��
	  case COERCION:												//COERCION�� ���
		  {
			  memcpy(&str_size, ptr, sizeof(str_size));										//str_size �� ���ڿ��� ���̸� ����
			  ptr = ptr + sizeof(str_size);													//str_size �� ũ�⸸ŭ ������ �̵�

			  memcpy(str, ptr, str_size);													//str �� ���ڿ� ����
			  ptr = ptr + str_size;															//str_size ��ŭ ������ �̵�

			  str[str_size] = '\0';															//���ڿ� ���� �ι���

			  printf("%s", str);															//���
			  ExitFlag = true;																//���� �÷��׸� �����
			  break;
		  }
	  }

	  if (ExitFlag)																			//������ �������Ƿ� Ŭ���̾�Ʈ�� �����մϴ�
	  {
		  break;
	  }
  }  

  closesocket(hSocket);																		//������ �ݰ�
  WSACleanup();																				//��� �����忡�� �����۾��� �����մϴ�
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