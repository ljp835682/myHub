#include "Tree.h"

int main()
{
	Tree * tree = new Tree;								//Ʈ�� ����
	tree->count = 0;									//ī��Ʈ �ʱ�ȭ
	tree->root = new Node;								//��Ʈ ����
	tree->root->left = nullptr;						
	tree->root->right = nullptr;						//�ʱ�ȭ

	int inputNodeCount;
	printf("��� ��带 �����Ͻðڽ��ϱ�? : ");
	scanf("%d", &inputNodeCount);						//��尹�� �Է�

	int intputData;
	for (int i = 0; i < inputNodeCount; i++)			
	{
		printf("�Է� : ");
		scanf("%d", &intputData);
		AddNode(tree, tree->root, intputData);			//��� �߰�
	}

	printf("���� ��ȸ�� : ");
	PreOrderTreePrint(tree, tree->root);				//���� ��ȸ�� ���
	printf("\n���� ��ȸ�� : ");
	InOrderTreePrint(tree, tree->root);					//���� ��ȸ�� ���
	printf("\n���� ��ȸ�� : ");
	PostOrderTreePrint(tree, tree->root);				//���� ǥ��� ���
	printf("\n");

	DestroyTree(tree, tree->root);						//Ʈ�� ����
}