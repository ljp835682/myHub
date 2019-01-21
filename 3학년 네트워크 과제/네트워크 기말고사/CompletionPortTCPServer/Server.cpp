#include "Server.h"
#include "LogManager.h"

Server * Server::inst = nullptr;

Server * Server::Getinst()
{
	if (inst == nullptr)
	{
		inst = new Server();
	}

	return inst;
}

void Server::Destroy()
{
	if (inst != nullptr)
	{
		delete inst;
	}
}

void Server::Init()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	HANDLE hThread;

	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; i++)
	{
		hThread = CreateThread(NULL, 0, WorkerThread, NULL, 0, NULL);

		if (hThread == NULL)
		{
			LogManager::err_quit("CreateThread(WorkerThread)");
		}

		CloseHandle(hThread);
	}

	Socket();
	Bind();
	Listen();
}

void Server::Run()
{
	SOCKET sock;
	SOCKADDR_IN addr;

	while (true)
	{
		if (Accept(sock, addr))
		{
			Client * ptr = ClientManager::GetInst()->AddClient(sock, addr);
			CreateIoCompletionPort((HANDLE)sock, hcp, sock, 0);

			if (!ptr->Recv())
			{
				ClientManager::GetInst()->RemoveClient(ptr);
				continue;
			}
		}
	}

	ClientManager::Destoroy();
}

DWORD WINAPI Server::WorkerThread(LPVOID arg)
{
	int retval;

	while (true)
	{
		DWORD cbTransferred;
		SOCKET client_sock;
		WSAOVERLAPPED overlap;
		retval = GetQueuedCompletionStatus(Server::Getinst()->hcp, &cbTransferred, (LPDWORD)&client_sock, (LPOVERLAPPED *)&overlap, INFINITE);
		Client * ptr = ClientManager::GetInst()->FindClient(client_sock);
		if (ptr == nullptr)
		{
			return 0;
		}

		ptr->SetOvelapped(overlap);

		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->GetSocket(), (SOCKADDR *)&clientaddr, &addrlen);

		if (retval == 0 || cbTransferred == 0)
		{
			//cbTransferred 이면 연결이 끊긴것, retval이 0이면 오류가 난것
			if (retval == 0)
			{
				DWORD temp1, temp2;
				ptr->GetOverlappedResult(temp1, temp2);
				LogManager::err_display("WSAGetOverlappedResult()");
			}

			closesocket(ptr->GetSocket());
			printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
			
			ClientManager::GetInst()->RemoveClient(ptr);
			continue;
		}

		ptr->StateSwitch(cbTransferred);

		if (ptr->GetEndFlag())
		{
			delete ptr->GetGameRoom();
			delete ptr;
		}
	}

	return 0;
}

void Server::Socket()
{
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (listen_sock == INVALID_SOCKET)
	{
		LogManager::err_quit("socket()");
	}
}

void Server::Bind()
{
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	int retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	
	if (retval == SOCKET_ERROR)
	{
		LogManager::err_quit("bind()");
	}
}

void Server::Listen()
{
	int retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) 
	{ 
		LogManager::err_quit("listen()");
	}
}

bool Server::Accept(SOCKET & _sock, SOCKADDR_IN & _addr)
{
	SOCKET sock;
	SOCKADDR_IN addr;

	int addrlen = sizeof(addr);
	sock = accept(listen_sock, (SOCKADDR *)&addr, &addrlen);
	
	if (sock == INVALID_SOCKET)
	{
		LogManager::err_display("accept()");
		return false;
	}

	_sock = sock;
	_addr = addr;

	return true;
}

Server::Server()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		LogManager::err_quit("WSAStartup()");
	}

	hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hcp == NULL)
	{
		LogManager::err_quit("CreateIoCompletionPort()");
	}
}

Server::~Server()
{
	WSACleanup();
}
