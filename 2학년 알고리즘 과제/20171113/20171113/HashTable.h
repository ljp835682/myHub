#include "Tree.h"

struct sHashTable				//�ؽ� ���̺� ����ü
{
	sTree ** tree;				//Ʈ���� ������ �迭(����ϴ°��� �����Ҵ��ϰ� ������� �ʴ°��� 4����Ʈ�� �Ҵ�)
};

sHashTable * CreateHashTable();						//�ؽ����̺� ���� �Լ�
sTree * CreateTree();								//Ʈ�� ���� �Լ�
sNode * Get(sHashTable * _hash, char * _str);		//��
void Set(sHashTable * _hash, char * _str);			//��
int KeyTrans(int _key);								//Ű ��ȯ
void DestroyHashTable(sHashTable * _hash);			//�ؽ� ���̺� ����

