#include "Client.h"
#include "LogManager.h"
#include "ClientManager.h"

Client::Client(SOCKET _sock, SOCKADDR_IN _addr)
{
	endFlag = false;

	hPartnerEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hPartnerEvent == NULL) 
	{
		LogManager::err_quit("CreateEvent() Error");
	}

	sock = _sock;
	addr = _addr;

	state = INIT_STATE;

	recvbytes = 0;
	comp_recvbytes = 0;
	sendbytes = 0;
	comp_sendbytes = 0;

	partner = nullptr;
	room = nullptr;

	ZeroMemory(&wsabuf, sizeof(WSABUF));
	ZeroMemory(&overlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(recvbuf, sizeof(recvbuf));
	ZeroMemory(sendbuf, sizeof(sendbuf));
	ZeroMemory(&point, sizeof(point));
	printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

Client::~Client()
{
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

bool Client::Recv()
{
	int retval;
	DWORD flags = 0;
	DWORD d_recvbytes;

	ZeroMemory(&overlapped, sizeof(overlapped));
	wsabuf.buf = recvbuf + comp_recvbytes;
	wsabuf.len = BUFSIZE - comp_recvbytes;

	retval = WSARecv(sock, &wsabuf, 1, &d_recvbytes,
		&flags, &overlapped, NULL);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LogManager::err_display("WSARecv()");
			//RemoveClientInfo(_ptr);
			return false;
		}
	}

	return true;
}

bool Client::Send(int _size)
{
	int retval;
	DWORD d_sendbytes;
	DWORD flags;

	ZeroMemory(&overlapped, sizeof(overlapped));

	if (sendbytes == 0)
	{
		sendbytes = _size;
	}

	wsabuf.buf = sendbuf + comp_sendbytes;
	wsabuf.len = sendbytes - comp_sendbytes;

	retval = WSASend(sock, &wsabuf, 1, &d_sendbytes,
		0, &overlapped, NULL);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LogManager::err_display("WSASend()");
			//RemoveClientInfo(_ptr);
			return false;
		}
	}

	return true;
}

int Client::OverlapSend(int _sendbyte)
{
	comp_sendbytes += _sendbyte;
	if (comp_sendbytes == sendbytes)
	{
		comp_sendbytes = 0;
		sendbytes = 0;

		return SOC_TRUE;
	}
	if (!Send(sendbytes))
	{
		return SOC_ERROR;
	}

	return SOC_FALSE;
}

int Client::OverlapRecv(int _recvbyte)
{
	if (recvbytes < sizeof(int))
	{
		comp_recvbytes += _recvbyte;

		if (comp_recvbytes >= sizeof(int))
		{
			memcpy(&recvbytes, recvbuf, sizeof(int));
			comp_recvbytes = comp_recvbytes - sizeof(int);
			memcpy(recvbuf, recvbuf + sizeof(int), comp_recvbytes);

			if (comp_recvbytes >= recvbytes)
			{
				comp_recvbytes -= recvbytes;
				return SOC_TRUE;
			}
			else
			{
				if (!Recv())
				{
					return SOC_ERROR;
				}
			}

		}
		else
		{
			if (!Recv())
			{
				return SOC_ERROR;
			}
		}
	}

	comp_recvbytes += _recvbyte;

	if (comp_recvbytes >= recvbytes)
	{
		comp_recvbytes -= recvbytes;
		return SOC_TRUE;
	}
	else
	{
		if (!Recv())
		{
			return SOC_ERROR;
		}

		return SOC_FALSE;
	}
}

STATE Client::GetState()
{
	return state;
}

void Client::PartnerMatchComple()
{
	SetEvent(hPartnerEvent); // 읽기 완료 알리기
	state = PACKET_RECV_STATE;
}

void Client::SetPartner(Client * _partner)
{
	partner = _partner;
}

void Client::PartnerRemove()
{
	endFlag = true;
}

Client * Client::GetPartner()
{
	return partner;
}

void Client::SetGameRoom(GameRoom * _room)
{
	room = _room;
}

GameRoom * Client::GetGameRoom()
{
	return room;
}

void Client::FirstState()
{
	wsabuf.buf = recvbuf;
	wsabuf.len = BUFSIZE;
	state = MATCHING_STATE;
}

bool Client::Equal(SOCKET _sock)
{
	if (sock == _sock)
	{
		return true;
	}

	return false;
}

void Client::SetOvelapped(WSAOVERLAPPED _overlapped)
{
	overlapped = _overlapped;
}

SOCKET Client::GetSocket()
{
	return sock;
}

void Client::GetOverlappedResult(DWORD & _temp1, DWORD & _temp2)
{
	WSAGetOverlappedResult(sock, &overlapped, &_temp1, FALSE, &_temp2);
}

void Client::StateSwitch(DWORD _cbTransferred)
{
	int retval;
	PROTOCOL protocol = INIT;

	if (state == MATCHING_STATE)
	{
		WaitForSingleObject(hPartnerEvent, INFINITE);
	}

	switch (state)
	{
	case PACKET_RECV_STATE:
		retval = OverlapRecv(_cbTransferred);
		switch (retval)
		{
		case SOC_ERROR:
			return;
		case SOC_FALSE:
			return;
		case SOC_TRUE:
			break;
		}

		GetProtocol(protocol);

		switch (protocol)
		{
		case TILE_SEND_REQ:
			state = TILE_SEND_STATE;
			break;

		case MOUSE_POINT:
			UnPackPacket(point.x, point.y);
			state = TILE_FLIP_STATE;
			break;
		}

		InitializeRecvBuffer();

		break;
	case RESULT_SEND_STATE:
		retval = OverlapSend(_cbTransferred);
		switch (retval)
		{
		case SOC_ERROR:
			return;
		case SOC_FALSE:
			return;
		case SOC_TRUE:
			break;
		}

		state = PACKET_RECV_STATE;

		if (endFlag)
		{
			return;
		}

		if (!Recv())
		{
			return;
		}

		break;
	}

	if (endFlag)
	{
		state = GAME_END_STATE;
	}

	int size;
	RESULT result = NODATA;
	protocol = INIT;
	switch (state)
	{
	case GAME_END_STATE:
		PackPacket(GAME_END, size);
		state = RESULT_SEND_STATE;

		if (!Send(size))
		{
			return;
		}
		break;

	case TILE_SEND_STATE:
		PackPacket(TILE_SEND, room, size);
		state = RESULT_SEND_STATE;

		if (!Send(size))
		{
			return;
		}
		break;

	case TILE_FLIP_STATE:
		int oldx = -1, oldy = 1;
		switch (room->ChoiceTile(this, point.x, point.y, oldx, oldy))
		{
		case 0:
			PackPacket(TILE_TEMP_FLIP, point.x, point.y, size);

			if (!Send(size))
			{
				return;
			}
			break;

			//뒤집기 성공
		case 1:
			PackPacket(TILE_FLIP_RESULT, TILE_FLIP_SUCCESE, point.x, point.y, oldx, oldy, size);

			if (!Send(size))
			{
				return;
			}
			break;

			//뒤집기 실패
		case 2:
			PackPacket(TILE_FLIP_RESULT, TILE_FLIP_FAIL, point.x, point.y, oldx, oldy, size);

			if (!Send(size))
			{
				return;
			}
			break;

		default:
			break;
		}


		break;
	}
}

bool Client::GetEndFlag()
{
	return endFlag;
}

void Client::GetProtocol(PROTOCOL & _protocol)
{
	memcpy(&_protocol, recvbuf, sizeof(_protocol));
}

void Client::UnPackPacket(int & _x, int & _y)
{
	char* ptr = recvbuf + sizeof(PROTOCOL);

	memcpy(&_x, ptr, sizeof(_x));
	ptr = ptr + sizeof(_x);

	memcpy(&_y, ptr, sizeof(_y));
	ptr = ptr + sizeof(_y);
}


void Client::PackPacket(PROTOCOL _protocol, GameRoom * _room, int & _size)
{
	char* ptr = sendbuf;
	_size = 0;

	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			Tile tile = _room->GetTiles(i, j);

			memcpy(ptr, &tile, sizeof(tile));
			ptr = ptr + sizeof(tile);
			_size = _size + sizeof(tile);
		}
	}

	ptr = sendbuf;
	memcpy(ptr, &_size, sizeof(_size));

	_size = _size + sizeof(_size);
}

void Client::PackPacket(PROTOCOL _protocol, int _x, int _y, int & _size)
{
	char* ptr = sendbuf;
	_size = 0;

	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);

	memcpy(ptr, &_x ,sizeof(_x));
	ptr = ptr + sizeof(_x);
	_size = _size + sizeof(_x);

	memcpy(ptr, &_y, sizeof(_y));
	ptr = ptr + sizeof(_y);
	_size = _size + sizeof(_y);

	ptr = sendbuf;
	memcpy(ptr, &_size, sizeof(_size));

	_size = _size + sizeof(_size);
}

void Client::PackPacket(PROTOCOL _protocol, RESULT _result, int _x1, int _y1, int _x2, int _y2, int & _size)
{
	char* ptr = sendbuf;
	_size = 0;

	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);

	memcpy(ptr, &_result, sizeof(_result));
	ptr = ptr + sizeof(_result);
	_size = _size + sizeof(_result);

	memcpy(ptr, &_x1, sizeof(_x1));
	ptr = ptr + sizeof(_x1);
	_size = _size + sizeof(_x1);

	memcpy(ptr, &_y1, sizeof(_y1));
	ptr = ptr + sizeof(_y1);
	_size = _size + sizeof(_y1);

	memcpy(ptr, &_x2, sizeof(_x2));
	ptr = ptr + sizeof(_x2);
	_size = _size + sizeof(_x2);

	memcpy(ptr, &_y2, sizeof(_y2));
	ptr = ptr + sizeof(_y2);
	_size = _size + sizeof(_y2);

	ptr = sendbuf;
	memcpy(ptr, &_size, sizeof(_size));

	_size = _size + sizeof(_size);
}

void Client::PackPacket(PROTOCOL _protocol, int & _size)
{
	char* ptr = sendbuf;
	_size = 0;

	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);

	ptr = sendbuf;
	memcpy(ptr, &_size, sizeof(_size));

	_size = _size + sizeof(_size);
}

void Client::InitializeRecvBuffer()
{
	memcpy(recvbuf, recvbuf + recvbytes, comp_recvbytes);
	recvbytes = 0;
}
