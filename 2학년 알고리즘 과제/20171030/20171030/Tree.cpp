#include "Tree.h"

//��带 �߰��ϴ� �Լ�
void AddNode(Tree * _tree, Node * _visit, int _data)
{
	if (_tree->count == 0)					//���� ù���� �Է½�
	{
		_visit->data = _data;				//�׳� ���Խ�ŵ�ϴ�
		_tree->count++;
		return;								//�Լ� ����
	}

	else									//�׿��� ���
	{		
		if (_visit->data > _data)			//���� �湮�� ����� �����ͺ��� �Էµ� �����Ͱ� �۴ٸ�
		{
			if (_visit->left == nullptr)	//���� �湮�� ����� ���� �ڽ��� �������� �ʴ´ٸ�
			{
				Node * newNode = new Node;
				newNode->data = _data;
				newNode->left = nullptr;
				newNode->right = nullptr;
				_visit->left = newNode;		//���ο� ��带 ����� �ű⿡ ����
				_tree->count++;
			}

			else							//���� �湮�� ����� ���� �ڽ��� �����Ǿ��ٸ�
			{
				AddNode(_tree, _visit->left, _data);
			}								//�ش� ���� �̵�
		}

		else if (_visit->data < _data)		//���� �湮�� ����� �����ͺ��� �Էµ� �����Ͱ� ũ�ٸ�
		{
			if (_visit->right == nullptr)	//���� �湮�� ����� ������ �ڽ��� �������� �ʴ´ٸ�
			{
				Node * newNode = new Node;
				newNode->data = _data;
				newNode->left = nullptr;
				newNode->right = nullptr;		
				_visit->right = newNode;	//���ο� ��带 ����� �ű⿡ ����
				_tree->count++;
			}

			else							//���� �湮�� ����� ������ �ڽ��� �����Ǿ��ٸ�
			{
				AddNode(_tree, _visit->right, _data);
			}								//�ش� ���� �̵�
		}
	}
}

//������ȸ ����Լ�
void PreOrderTreePrint(Tree * _tree, Node * _visit)
{
	if (_visit == nullptr)							//���� �湮�� ��尡 �������� �ʴ� ���� �Լ�����
	{
		return;
	}

	else
	{
		printf(" %d ", _visit->data);				//�ڽ��� ���� ���
		PreOrderTreePrint(_tree, _visit->left);		//�ڽ��� ���� �ڽ��� ����ϱ����� ����Լ� ȣ��
		PreOrderTreePrint(_tree, _visit->right);	//�ڽ��� ������ �ڽ��� ����ϱ����� ����Լ� ȣ��
	}
}

//������ȸ ����Լ�
void InOrderTreePrint(Tree * _tree, Node * _visit)
{
	if (_visit == nullptr)							//���� �湮�� ��尡 �������� �ʴ� ���� �Լ�����
	{
		return;
	}

	else
	{
		InOrderTreePrint(_tree, _visit->left);		//�ڽ��� ���� �ڽ��� ����ϱ����� ����Լ� ȣ��
		printf(" %d ", _visit->data);				//�ڽ��� �߾ӿ� ���
		InOrderTreePrint(_tree, _visit->right);		//�ڽ��� ������ �ڽ��� ����ϱ����� ����Լ� ȣ��
	}
}

//������ȸ ����Լ�
void PostOrderTreePrint(Tree * _tree, Node * _visit)
{
	if (_visit == nullptr)							//���� �湮�� ��尡 �������� �ʴ� ���� �Լ�����
	{	
		return;
	}

	else
	{
		PostOrderTreePrint(_tree, _visit->left);	//�ڽ��� ���� �ڽ��� ����ϱ����� ����Լ� ȣ��
		PostOrderTreePrint(_tree, _visit->right);	//�ڽ��� ������ �ڽ��� ����ϱ����� ����Լ� ȣ��
		printf(" %d ", _visit->data);				//�ڽ��� �������� ���
	}
}

//Ʈ�������Լ�
void DestroyTree(Tree * _tree, Node * _visit)
{
	if (_visit == nullptr)							//���� �湮�� ��尡 �������� �ʴ� ���� �Լ�����
	{
		return;
	}
													//Ʈ���� ������ �Ϸ��� �ǾƷ��� ������ ������ �ʿ䰡 �����Ƿ�
													//������ȸ�� �Ͽ� ������ �ϴ°��� �����ϴ�
	else
	{
		DestroyTree(_tree, _visit->left);			//�ڽ��� ���� �ڽ��� �����ϱ����� ����Լ� ȣ��		
		DestroyTree(_tree, _visit->right);			//�ڽ��� ������ �ڽ��� �����ϱ����� ����Լ� ȣ��
		delete _visit;								//�ڽ��� �������� ����
		_visit = nullptr;
	}									
}