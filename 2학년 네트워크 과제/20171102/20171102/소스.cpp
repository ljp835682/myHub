#include <iostream>
#include <fstream>

using namespace std;

int main()
{
	ofstream out;										//파일을 오픈할 개체
	out.open("output.txt", ios::in | ios::ate);			//지정 파일 오픈, 열수있게,맨뒤에 입력

	char str1[100] = "";								//임시 저장 문자열

	for (int i = 0; i < 5; i++)
	{
		cin >> str1;
		out << str1 << endl;							//5줄 입력
	}

	out.close();										//반환

	ifstream in("output.txt");							//파일을 오픈할 개체
	char str2[100] = "";								//임시 저장 문자열

	while (!in.eof())									//파일의 끝이 아닐떄동안 반복
	{
		in.getline(str2, 100);							//파일의 한줄을 읽어옴
		cout << str2 << endl;							//출력
	}

	in.close();											//반환
	return 0;
}