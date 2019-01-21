#pragma once

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <iostream>

using std::cout;
using std::cin;

#define BUFSIZE 512
#define PLAYERCOUNT 2
#define MAXUSER   10
#define NODATA   -1

enum { NO_WIN, FIRST_WIN, SECOND_WIN = -1 };
enum PROTOCOL { INTRO, STRING, CREATEROOM, FINDROOM, CONNECTED };

