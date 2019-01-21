#include <iostream>
using namespace std;

#define STRSIZE 30
#define OK 1
#define MISS 2
#define NO 3

class cUser
{
private:
	char ID[STRSIZE];
	char PW[STRSIZE];

public:
	void SetIDPW(char * _ID, char * _PW)
	{
		strcpy_s(ID, strlen(_ID) + 1, _ID);
		strcpy_s(PW, strlen(_PW) + 1, _PW);
	}

	char * GetID()
	{
		return ID;
	}
	char * GetPW()
	{
		return PW;
	}

	int LoginCheck(char * _ID, char * _PW)
	{
		if ((strcmp(_ID, ID) == 0) && ((strcmp(_PW, PW) == 0)))
		{
			return OK;
		}

		else if ((strcmp(_ID, ID) == 0) && ((strcmp(_PW, PW) != 0)))
		{
			return MISS;
		}

		else
		{
			return NO;
		}
	}
};