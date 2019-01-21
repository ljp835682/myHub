#include "HashTable.h"

int main()
{
	sHashTable * hash = CreateHashTable();					//����
	sNode * node = nullptr;									//�˻���
	while (1)
	{
		char str[30];
		bool exit = false;
		int num;
		printf("1. �Է�, 2. �˻�, 3. ���� : ");
		scanf("%d", &num);

		switch (num)
		{
		case 1:
			scanf("%s", str);
			Set(hash, str);									//�Է�
			break;
		case 2:
			scanf("%s", str);
			node = Get(hash, str);							//�˻�

			if (node == nullptr)							//���� ����
			{
				printf("null\n");								
				break;							
			}

															//���
			printf("���ڿ� : %s, Ű : %d, ����Ű : %d\n", node->str, node->key, node->hiddenkey);
			break;											
		case 3:
			exit = true;
			break;
		}

		if (exit)
		{
			DestroyHashTable(hash);
			return 0;
		}
	}
}