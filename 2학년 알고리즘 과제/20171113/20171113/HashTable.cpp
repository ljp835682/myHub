#include "HashTable.h"

//Ű ��ȯ �Լ�
int KeyTrans(int _key)					
{
	return _key % 199;						//�Ҽ� 199 �̸��� ���� ��ȯ
}

//�ؽ����̺� ���� �Լ�
sHashTable * CreateHashTable()
{
	sHashTable * hash = new sHashTable;		//�ؽ�����
	hash->tree = new sTree *[199];			//199ĭ �迭����

	for (int i = 0; i < 199; i++)			
	{
		hash->tree[i] = nullptr;			//���� �ʱ�ȭ
	}

	return hash;
}

//Ʈ�� ���� �Լ�
sTree * CreateTree()						
{
	sTree * tree = new sTree;				//Ʈ�� ����
	tree->root = nullptr;					
	tree->count = 0;						//�ʱ�ȭ

	return tree;
}

//�Է� �Լ�
void Set(sHashTable * _hash, char * _str)											
{
	sNode * newNode = CreateNode(_str);										//���ο� ��� ����
	int index = KeyTrans(newNode->key);										//Ű ��ȯ
	
	if (_hash->tree[index] == nullptr)										//���� �ش� Ʈ���� ù��° �����
	{
		_hash->tree[index] = CreateTree();									//�ϴ� Ʈ���� ����
	}

	AddNode(_hash->tree[index], _hash->tree[index]->root, newNode);			//�׸��� Ʈ���� ���ο� ��带 ��
}

//��� �Լ�
sNode * Get(sHashTable * _hash, char * _str)
{
	int index = KeyTrans(Key(_str));														//���ڿ��� �������� ����Ǿ����� ������ ã�Ƴ�
	sNode * node = nullptr;

	if (_hash->tree[index] != nullptr)														//��������
	{
		node = SearchNode(_hash->tree[index], _hash->tree[index]->root, HiddenKey(_str));	//�������� ������ ã�� ����

		if (node != nullptr)																//ã�µ� �����ߴٸ�
		{
			return node;																	//����
		}
	}

	return nullptr;																			//����
}

void DestroyHashTable(sHashTable * _hash)													//�ؽ� ���̺� ����
{
	for (int i = 0; i < 199; i++)
	{
		if (_hash->tree[i] != nullptr)														//���� ���� ����
		{
			DestroyTree(_hash->tree[i], _hash->tree[i]->root);								//Ʈ���� ��� ����
			delete _hash->tree[i];															//Ʈ�� ����
		}
	}

	delete _hash->tree;																		//Ʈ���迭 ����
	delete _hash;																			//�ؽ����̺� ����
}