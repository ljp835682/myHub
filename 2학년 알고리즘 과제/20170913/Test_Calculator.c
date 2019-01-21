#include <stdio.h>
#include <string.h>

#define STRINGLEN 100

#define PUL			1
#define MIN			2
#define MUL			3
#define DEV			4
#define LPARENT		5
#define RPARENT		6

void TastebufSetting(char * _InfixNotation, char * _Tastebuf)
{
	int i = 0;
	int j = 0;

	while (i != strlen(_InfixNotation) + 1)
	{
		if (_InfixNotation[i] == '+' || _InfixNotation[i] == '-')
		{
			if (j != 0)
			{
				if (_Tastebuf[j - 1] == '*')
				{
					_Tastebuf[j - 1] = _InfixNotation[i];			//팝
					_Tastebuf[j] = '*';								//푸쉬
				}

				else if (_Tastebuf[j - 1] == '/')
				{
					_Tastebuf[j - 1] = _InfixNotation[i];			//팝
					_Tastebuf[j] = '/';								//푸쉬
				}

				else if ((_Tastebuf[j - 1] == '+') || (_Tastebuf[j - 1] == '-'))
				{
					_Tastebuf[j] = _InfixNotation[i];
				}
			}

			else
			{
				_Tastebuf[j] = _InfixNotation[i];
			}

			j++;
		}

		else if (_InfixNotation[i] == '*' || _InfixNotation[i] == '/')
		{
			_Tastebuf[j] = _InfixNotation[i];
			j++;
		}

		i++;
	}
}


int main( void )
{
	char InfixNotation[STRINGLEN] = "";		//중위표기식 문자열
	char PostfixNotation[STRINGLEN] = "";	//후위표기식 문자열

	printf("중위표기식 입력 : ");
	scanf("%s", InfixNotation);

	//int InfixLen = strlen(InfixNotation);		//중위표기식 문자열 길이
	char Tastebuf[STRINGLEN] = "";

	/*중위 표기식->기호버퍼*/
	TastebufSetting(&InfixNotation, &Tastebuf);
	printf("연산자 우선도 : %s", Tastebuf);

	/*중위 표기식->숫자버퍼*/
	

    return 0;

	//----------------------------------------------
	
	//과제2

	//char InputString[STRINGLEN] = "";

	//printf("입력 : ");
	//scanf("%s", InputString);

	//char Stack[STRINGLEN] = "";

	//int j = 0;
	//for (int i = strlen(InputString) - 1; i >= 0; i--)
	//{
	//	Stack[j] = InputString[i];
	//	j++;
	//}

	//printf("%s\n", Stack);

	//----------------------------------------------
}

/*

1+2*3-1

1.x
2.+ buf1
3.x
4.*	buf0
5.x
6.- buf2
7.x

*/