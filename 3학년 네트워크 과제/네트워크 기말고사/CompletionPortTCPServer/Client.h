#pragma once
#include "Global.h"
#include "GameRoom.h"

enum RESULT
{
	NODATA,
	TILE_FLIP_SUCCESE,
	TILE_FLIP_FAIL,
};

enum PROTOCOL
{
	INIT,
	GAME_END,
	TILE_SEND_REQ,
	TILE_SEND,
	MOUSE_POINT,
	TILE_FLIP_RESULT,
	TILE_TEMP_FLIP,
};

enum STATE
{
	INIT_STATE,
	MATCHING_STATE,
	GAME_INIT_STATE,
	TILE_SEND_STATE,
	PACKET_RECV_STATE,
	RESULT_SEND_STATE,
	GAME_END_STATE,
	TILE_FLIP_STATE,
};

enum
{
	SOC_ERROR = 1, SOC_TRUE, SOC_FALSE
};

struct MousePoint
{
	int x;
	int y;
};

class Client
{
private:
	WSAOVERLAPPED overlapped;

	SOCKET sock;
	SOCKADDR_IN addr;
	WSABUF wsabuf;
	STATE state;

	GameRoom * room;

	int recvbytes;
	int comp_recvbytes;
	int sendbytes;
	int comp_sendbytes;

	char recvbuf[BUFSIZE];
	char sendbuf[BUFSIZE];

	HANDLE hPartnerEvent;
	Client * partner;
	bool endFlag;

	MousePoint point;

public:
	Client(SOCKET _sock, SOCKADDR_IN _addr);
	~Client();

	bool Recv();
	bool Send(int _size);

	int OverlapSend(int _sendbyte);
	int OverlapRecv(int _recvbyte);

	void PartnerMatchComple();
	void SetPartner(Client * _partner);
	void PartnerRemove();
	Client * GetPartner();

	void SetGameRoom(GameRoom * _room);
	GameRoom * GetGameRoom();

	void FirstState();
	bool Equal (SOCKET _sock);

	void SetOvelapped(WSAOVERLAPPED _overlapped);
	SOCKET GetSocket();

	void GetOverlappedResult(DWORD & _temp1, DWORD & _temp2);

	STATE GetState();
	void StateSwitch(DWORD _cbTransferred);
	bool GetEndFlag();

	void GetProtocol(PROTOCOL & _protocol);
	void UnPackPacket(int & _x,int & _y);
	void PackPacket(PROTOCOL _protocol, GameRoom * _room, int & _size);
	void PackPacket(PROTOCOL _protocol, int _x, int _y, int & _size);
	void PackPacket(PROTOCOL _protocol, RESULT _result, int _x1, int _y1, int _x2, int _y2, int & _size);
	void PackPacket(PROTOCOL _protocol, int & _size);

	void InitializeRecvBuffer();
};

