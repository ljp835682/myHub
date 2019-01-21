#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"		//������ (�ڱ��ڽ����� ���ƿ��� �ּ�)
#define SERVERPORT 9000				//���� ��Ʈ
#define BUFSIZE    512

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while(left > 0)
	{
		received = recv(s, ptr, left, flags);	//���Ͽ��� �����͸� �����ϴ� �Լ�
		if(received == SOCKET_ERROR)			//�����ϰ��
			return SOCKET_ERROR;				
		else if(received == 0)					
			break;
		left -= received;						//������ŭ ��
		ptr += received;						//������ ��ġ�� ������ �����͸�ŭ �ű�
	}

	return (len - left);						//��ǥ��  len���� ���� �뷮�� ��
}												//��Ȯ�� �� ����Ʈ�� �޾ƾ����� �˶��� ���

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);			//���� ���� �κ�
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);		//���� ������
	serveraddr.sin_port = htons(SERVERPORT);				//���� ��Ʈ
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
															//������ ���� �������� serveraddr�� ����� �������� ������ ��û
															//bind�� ��Ȱ�� connect�� ���ָ� OS�� �缳 ��Ʈ��ȣ�� ������� �ƴѰ� �ϳ��� �ڵ����� �Ҵ���
															//connect�� ���� ��û ��Ŷ�� ������
	if(retval == SOCKET_ERROR) err_quit("connect()");		

	// ������ ��ſ� ����� ����
	char buf[BUFSIZE+1];
	int len;

	// ������ ������ ���
	while(1){
		// ������ �Է�
		printf("\n[���� ������] ");
		if(fgets(buf, BUFSIZE+1, stdin) == NULL)		//fget�� file���� ���ھ �о ���ۿ��� ���� �Լ�
			break;

		// '\n' ���� ����
		len = strlen(buf);
		if(buf[len-1] == '\n')
			buf[len-1] = '\0';							//�����Է½� closesocket���� �̵�
		if(strlen(buf) == 0)
			break;

		// ������ ������
		retval = send(sock, buf, strlen(buf), 0);		//���ۿ� ���� ���̸�ŭ�� ��� ���� ���Ͽ� ����
		if(retval == SOCKET_ERROR){
			err_display("send()");
			break;
		}
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n", retval);	//������ �۽� ���ۿ� ���⸸��, ���� ���Ἲ���� �ƴ�
	
		// ������ �ޱ�
		retval = recvn(sock, buf, retval, 0);			//send�� ������ŭ �޾Ƴ������� retval�� ���
		if(retval == SOCKET_ERROR){
			err_display("recv()");
			break;
		}
		else if(retval == 0)
			break;

		// ���� ������ ���
		buf[retval] = '\0';
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\n", retval);
		printf("[���� ������] %s\n", buf);
	}

	// closesocket()
	closesocket(sock);				

	// ���� ����
	WSACleanup();
	return 0;
}