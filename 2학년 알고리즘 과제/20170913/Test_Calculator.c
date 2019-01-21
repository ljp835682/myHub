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
					_Tastebuf[j - 1] = _InfixNotation[i];			//��
					_Tastebuf[j] = '*';								//Ǫ��
				}

				else if (_Tastebuf[j - 1] == '/')
				{
					_Tastebuf[j - 1] = _InfixNotation[i];			//��
					_Tastebuf[j] = '/';								//Ǫ��
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
	char InfixNotation[STRINGLEN] = "";		//����ǥ��� ���ڿ�
	char PostfixNotation[STRINGLEN] = "";	//����ǥ��� ���ڿ�

	printf("����ǥ��� �Է� : ");
	scanf("%s", InfixNotation);

	//int InfixLen = strlen(InfixNotation);		//����ǥ��� ���ڿ� ����
	char Tastebuf[STRINGLEN] = "";

	/*���� ǥ���->��ȣ����*/
	TastebufSetting(&InfixNotation, &Tastebuf);
	printf("������ �켱�� : %s", Tastebuf);

	/*���� ǥ���->���ڹ���*/
	

    return 0;

	//----------------------------------------------
	
	//����2

	//char InputString[STRINGLEN] = "";

	//printf("�Է� : ");
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