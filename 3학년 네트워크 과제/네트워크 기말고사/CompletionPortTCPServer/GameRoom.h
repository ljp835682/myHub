#pragma once
#include "Global.h"

enum TILESTATE { HIDDEN, FLIP, TEMPFLIP };
enum GAMESTATE { RUN, HINT, VIEW };

class Client;

struct Tile
{
	int num;
	TILESTATE state;
};

struct Player
{
	Client * client;
	int count;
};

class GameRoom
{
private:
	GAMESTATE state;
	Tile tile[4][4];
	int turnUserIndex;
	Player player[2];

	bool FindFlipTile(int & _x, int & _y);

public:
	GameRoom(Client * _player1, Client * _player2);
	~GameRoom();

	int ChoiceTile(Client * _player, int _x1, int _y1, int & _x2, int & _y2);
	Tile GetTiles(int _index1, int _index2);
};