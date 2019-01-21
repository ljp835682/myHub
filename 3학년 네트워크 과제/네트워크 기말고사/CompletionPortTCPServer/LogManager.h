#pragma once
#include "Global.h"

class LogManager
{
private:
	LogManager();
	~LogManager();

public:
	static void err_quit(char *msg);
	static void err_display(char *msg);
};

