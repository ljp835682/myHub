#pragma once
#include <list>
#include "Client.h"
#include "GameRoom.h"
using std::list;

class ClientManager
{
private:

	static ClientManager * inst;
	list<Client *> * clientList;
	CRITICAL_SECTION cs;

	ClientManager();
	~ClientManager();

public:
	static ClientManager * GetInst();
	static void Destoroy();

	Client * FindClient(SOCKET _sock);
	Client * AddClient(SOCKET _sock, SOCKADDR_IN _addr);
	void RemoveClient(Client * _ptr);
	bool Matching(Client * _ptr);
};

