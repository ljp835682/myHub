#include "GameRoom.h"
#define random(n) (rand()%n)

GameRoom::GameRoom(Client * _player1, Client * _player2)
{
	turnUserIndex = 0;

	player[0].client = _player1;
	player[1].client = _player2;
	
	for (int i = 0; i < 2; i++)
	{
		player[i].count = 0;
	}

	ZeroMemory(tile, sizeof(tile));

	int x, y;

	for (int i = 0; i <= 8; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			do {
				x = random(4);
				y = random(4);
			} while (tile[x][y].num != 0);
			tile[x][y].num = i;
		}
	}

	state = HINT;
}


GameRoom::~GameRoom()
{
}

int GameRoom::ChoiceTile(Client * _player, int _x1, int _y1, int & _x2, int & _y2)
{
	int nx = _x1 / 64;
	int ny = _y1 / 64;

	if (_player != player[turnUserIndex].client)
	{
		return -1;
	}

	if (nx > 3 || ny >= 3 || state != RUN)
	{
		return -1;
	}

	if (tile[nx][ny].state != HIDDEN)
	{
		return -1;
	}

	int tx = -1, ty = -1;
	FindFlipTile(tx, ty);

	if (tx == -1 && ty == -1)
	{
		tile[nx][ny].state = TEMPFLIP;
		return 0;
	}

	player[turnUserIndex].count++;
	_x2 = tx;
	_y2 = ty;

	if (tile[tx][ty].num == tile[nx][ny].num)
	{
		tile[tx][ty].state = FLIP;
		tile[nx][ny].state = FLIP;
		return 1;
	}

	else
	{
		tile[tx][ty].state = HIDDEN;
		tile[nx][ny].state = HIDDEN;
		turnUserIndex = (turnUserIndex == 0) ? 1 : 0;
		return 2;
	}

	return -1;
}

Tile GameRoom::GetTiles(int _index1, int _index2)
{
	return tile[_index1][_index2];
}

bool GameRoom::FindFlipTile(int & _x, int & _y)
{
	for (int i = 0; i<4; i++) 
	{
		for (int j = 0; j<4; j++) 
		{
			if (tile[i][j].state == TEMPFLIP) 
			{
				_x = i;
				_y = j;
				return true;
			}
		}
	}
	
	return false;
}
