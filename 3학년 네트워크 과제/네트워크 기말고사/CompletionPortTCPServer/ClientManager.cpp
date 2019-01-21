#include "ClientManager.h"

ClientManager * ClientManager::inst = nullptr;

ClientManager::ClientManager()
{
	InitializeCriticalSection(&cs);
	clientList = new list<Client *>();
}

ClientManager::~ClientManager()
{
	DeleteCriticalSection(&cs);
	delete clientList;
}

ClientManager * ClientManager::GetInst()
{
	if (inst == nullptr)
	{
		inst = new ClientManager();
	}

	return inst;
}

void ClientManager::Destoroy()
{
	if (inst != nullptr)
	{
		delete inst;
	}
}

Client * ClientManager::FindClient(SOCKET _sock)
{
	for (list<Client *>::iterator iter = clientList->begin(); iter != clientList->end(); iter++)
	{
		if ((*iter)->Equal(_sock))
		{
			return (*iter);
		}
	}

	return nullptr;
}

Client * ClientManager::AddClient(SOCKET _sock, SOCKADDR_IN _addr)
{
	EnterCriticalSection(&cs);

	Client * ptr = new Client(_sock, _addr);
	ptr->FirstState();

	Matching(ptr);
	clientList->push_back(ptr);


	LeaveCriticalSection(&cs);

	return ptr;
}

void ClientManager::RemoveClient(Client * _ptr)
{
	EnterCriticalSection(&cs);

	clientList->remove(_ptr);
	clientList->remove(_ptr->GetPartner());
	_ptr->GetPartner()->PartnerRemove();
	_ptr->SetPartner(nullptr);

	delete _ptr;

	LeaveCriticalSection(&cs);
}

bool ClientManager::Matching(Client * _ptr)
{
	for (list<Client *>::iterator iter = clientList->begin(); iter != clientList->end(); iter++)
	{
		if ((*iter)->GetPartner() == nullptr && (*iter) != _ptr)
		{
			(*iter)->SetPartner(_ptr);
			_ptr->SetPartner((*iter));

			GameRoom * newRoom = new GameRoom((*iter), _ptr);
			(*iter)->SetGameRoom(newRoom);
			_ptr->SetGameRoom(newRoom);

			(*iter)->PartnerMatchComple();
			_ptr->PartnerMatchComple();
			return true;
		}
	}

	return false;
}
