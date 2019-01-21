#pragma once
#include "Global.h"

class _Node;

struct _UserInfo
{
	char id[BUFSIZE];
	char pw[BUFSIZE];
	_Node * joinRoom;
};

struct _ClientInfo
{
	_UserInfo userinfo;
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	char  packetbuf[BUFSIZE];
	HANDLE hthread;
	HANDLE eventHandle;							//먼저 입력받고 나가는거 방지
	HANDLE catchHandle;							//중간에 나가고나서 생기는 오류 방지
	HANDLE endhandle;							//만약의 먼저 반환하는 상황을 방지
};

_ClientInfo* ClientInfo[MAXUSER];
HANDLE hThread[MAXUSER];

int count = 0;
int threadcount = 0;

class _Node
{
private:
	char id[BUFSIZE];
	char pw[BUFSIZE];
	_ClientInfo ** client;

	int userCount;
	int maxCount;

public:
	_Node * prev;
	_Node * next;

	_Node(int _count, _ClientInfo * _creater, char * _id, char * _pw)
	{
		maxCount = _count;
		client = new _ClientInfo * [maxCount + 1];
		client[0] = _creater;
		client[0]->userinfo.joinRoom = this;

		for (int i = 1; i < maxCount; i++)
		{
			client[i] = nullptr;
		}

		strcpy_s(id, _id);
		strcpy_s(pw, _pw);

		prev = nullptr;
		next = nullptr;
		userCount = 1;
	}

	~_Node()
	{
		delete client;
	}

	bool SendString(char * buf,int size)
	{
		int retval = 0;

		for (int i = 0; i < userCount; i++)
		{
			retval = send(client[i]->sock, buf, size, 0);
			if (retval == SOCKET_ERROR) 
			{
				err_display("send()");
				break;
			}
		}
	}

	bool AddClient(_ClientInfo * _client, char * _id, char * _pw)
	{
		if (strcmp(id, _id) != 0 && strcmp(pw, _pw) != 0)
		{
			return false;
		}

		client[userCount] = _client;
		client[userCount]->userinfo.joinRoom = this;
		userCount++;

		return true;
	}

	bool RemoveCilent(_ClientInfo * _client)
	{
		for (int i = 0; i < userCount; i++)
		{
			if (client[i] == _client)
			{
				for (int j = i; j < userCount; j++)
				{
					client[j] = client[j + 1];
				}

				break;
			}
		}
	}

	bool isEmpty()
	{
		return (userCount == 0) ? true : false;	//이걸 클라가나가면 이걸 웨이팅스레드에서 호출해서
	}											//true일경우 리스트에서 노드를 삭제시키고
												//아닐경우 그대로둠
};