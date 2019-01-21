#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRINGLEN 40

struct sNode
{
	int key;					//�ڽ��� �ּҸ� ���� �ڽ��� Ű
	int hiddenkey;				//�浹�� ����� �ڽ��� ����Ű (����Ȥ�� ���� Ȥ�� Ư����ȣ�� �޴��� ���� �ٸ� ���ڿ��� ���� ���� ���ü�����)
	char str[STRINGLEN];		//����� ���ڿ�
	sNode * left;				//�ڽ��� ���� �ڽ� ���
	sNode * right;				//�ڽ��� ������ �ڽ� ���
};

int Key(char * _str);				//Ű ���� �Լ�
int HiddenKey(char * _str);			//����Ű ���� �Լ�
sNode * CreateNode(char * _str);	//��� ���� �Լ�

struct sTree						//Ʈ�� ����ü
{
	sNode * root;					//��Ʈ ���
	int count;						//����� ����
};

void AddNode(sTree * _tree, sNode * _visit, sNode * _newNode);		//��带 �߰��ϴ� �Լ�
sNode * SearchNode(sTree * _tree, sNode * _visit, int _hiddenKey);	//��� Ž��
void DestroyTree(sTree * _tree, sNode * _visit);					//Ʈ�������Լ�