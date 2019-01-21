#include "Server.h"
#include "Global.h"

int main()
{
	Server::Getinst()->Init();
	Server::Getinst()->Run();
	Server::Destroy();
}