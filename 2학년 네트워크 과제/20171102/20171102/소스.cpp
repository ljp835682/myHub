#include <iostream>
#include <fstream>

using namespace std;

int main()
{
	ofstream out;										//������ ������ ��ü
	out.open("output.txt", ios::in | ios::ate);			//���� ���� ����, �����ְ�,�ǵڿ� �Է�

	char str1[100] = "";								//�ӽ� ���� ���ڿ�

	for (int i = 0; i < 5; i++)
	{
		cin >> str1;
		out << str1 << endl;							//5�� �Է�
	}

	out.close();										//��ȯ

	ifstream in("output.txt");							//������ ������ ��ü
	char str2[100] = "";								//�ӽ� ���� ���ڿ�

	while (!in.eof())									//������ ���� �ƴҋ����� �ݺ�
	{
		in.getline(str2, 100);							//������ ������ �о��
		cout << str2 << endl;							//���
	}

	in.close();											//��ȯ
	return 0;
}