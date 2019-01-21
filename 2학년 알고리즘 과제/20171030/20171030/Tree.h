#include <stdio.h>
#include <stdlib.h>

struct Node						//��� ����ü
{	
	int data;
	Node * left;				//�ڽ��� ���� �ڽ� ���
	Node * right;				//�ڽ��� ������ �ڽ� ���
};

struct Tree						//Ʈ�� ����ü
{
	Node * root;				//��Ʈ ���
	int count;					//����� ����
};

void AddNode(Tree * _tree, Node * _visit, int _data);		//��带 �߰��ϴ� �Լ�
void PreOrderTreePrint(Tree * _tree, Node * _visit);		//������ȸ ����Լ�
void InOrderTreePrint(Tree * _tree, Node * _visit);			//������ȸ ����Լ�
void PostOrderTreePrint(Tree * _tree, Node * _visit);		//������ȸ ����Լ�
void DestroyTree(Tree * _tree, Node * _visit);				//Ʈ�������Լ�