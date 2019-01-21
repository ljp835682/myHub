#pragma once
#include "Global.h"
#include "ClientManager.h"

class Server
{
private:
	static Server * inst;
	HANDLE hcp;
	SOCKET listen_sock;
	SOCKADDR_IN serveraddr;

	Server();
	~Server();

	static DWORD WINAPI WorkerThread(LPVOID arg);
	void Socket();
	void Bind();
	void Listen();
	bool Accept(SOCKET & _sock, SOCKADDR_IN & _addr);

public:
	static Server * Getinst();
	static void Destroy();
	
	void Init();
	void Run();
};

