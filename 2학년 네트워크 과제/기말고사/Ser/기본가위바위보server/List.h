#pragma once
#include "Node.h"

class _List
{
private:
	_Node * room;
	int roomCount;

public:
	_List()
	{
		room = nullptr;
		roomCount = 0;
	}

	bool isEmpty()
	{
		return (roomCount == 0) ? true : false;
	}

	void AddRoom(_Node * node)
	{
		if (room == nullptr)
		{
			room = node;
		}

		else
		{
			node->next = room;
			room->prev = node;
			room = node;
		}
	}

	void RemoveRoom(_Node * _find)
	{
		_Node * ptr = room;

		while (ptr != nullptr)
		{
			if (ptr == _find)
			{
				delete ptr;
			}

			ptr = ptr->next;
		}
	}

	_Node * FindRoom(_Node * _find)
	{
		_Node * ptr = room;

		while (ptr != nullptr)
		{
			if (ptr == _find)
			{
				break;
			}

			ptr = ptr->next;
		}

		return ptr;
	}
};