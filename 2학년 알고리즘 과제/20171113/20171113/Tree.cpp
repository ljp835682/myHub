#include "Tree.h"

//Ű ���� �Լ�
int Key(char * _str)								
{
	int newKey = 0;

	for (int i = 0; i < strlen(_str); i++)
	{
		newKey += _str[i];							//�����ڵ带 ���� ����
	}

	return newKey;
}

//����Ű ���� �Լ�
int HiddenKey(char * _str)							
{
	char str[255] = "";
	char temp[10] = "";

	for (int i = 0; i < strlen(_str); i++)			
	{
		sprintf(temp, "%d", int(_str[i]));			//�����ڵ带 ���� �̾������ �ϳ��� ���ڷ� ����
		strcat(str, temp);							//�� ) ���ڿ� "abc" -> ���� 979899
	}												//�ʹ� ū ���� �ؽ����̺��� �����������

	return atoi(str);								
}

//��� ���� �Լ�
sNode * CreateNode(char * _str)						
{
	sNode * newNode = new sNode;			

	strcpy_s(newNode->str, _str);
	newNode->left = nullptr;
	newNode->right = nullptr;
	newNode->key = Key(_str);
	newNode->hiddenkey = HiddenKey(_str);			//���� �ʱ�ȭ

	return newNode;	
}

//��带 �߰��ϴ� �Լ�
void AddNode(sTree * _tree, sNode * _visit, sNode * _newNode)
{
	if (_tree->count == 0)									//���� ù��° �Է½�
	{
		_tree->root = _newNode;
		_tree->count++;
		return;												//�Լ� ����
	}

	else													//�׿��� ���
	{
		if (_visit->hiddenkey > _newNode->hiddenkey)		//����Ű���� �������
		{
			if (_visit->left == nullptr)
			{
				_visit->left = _newNode;					//�Է�
				_tree->count++;
			}

			else
			{
				AddNode(_tree, _visit->left, _newNode);		//�̹� �ִ� ����ϰ�� �� ��� �Ʒ���
			}
		}

		else if (_visit->hiddenkey < _newNode->hiddenkey)	//����Ű���� Ŭ���
		{
			if (_visit->right == nullptr)
			{
				_visit->right = _newNode;					//�Է�
				_tree->count++;
			}

			else
			{
				AddNode(_tree, _visit->right, _newNode);	//�̹� �ִ� ����ϰ�� �� ��� �Ʒ���
			}
		}
	}
}

//��� �˻� �Լ�
sNode * SearchNode(sTree * _tree, sNode * _visit, int _hiddenKey)
{
	if (_visit == nullptr)
	{
		return nullptr;
	}

	else
	{
		if (_hiddenKey == _visit->hiddenkey)						//����Ű�� �������
		{
			return _visit;											//�Լ�����
		}

		else if (_visit->hiddenkey > _hiddenKey)					//�������
		{	
			return SearchNode(_tree, _visit->left, _hiddenKey);		//���� ���� �̵�
		}

		else if (_visit->hiddenkey < _hiddenKey)					//Ŭ���
		{
			return SearchNode(_tree, _visit->right, _hiddenKey);	//������ ���� �̵�
		}
	}
}

//Ʈ�������Լ�
void DestroyTree(sTree * _tree, sNode * _visit)
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