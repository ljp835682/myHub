#include "HashTable.h"

int main()
{
	sHashTable * hash = CreateHashTable();					//생성
	sNode * node = nullptr;									//검색용
	while (1)
	{
		char str[30];
		bool exit = false;
		int num;
		printf("1. 입력, 2. 검색, 3. 종료 : ");
		scanf("%d", &num);

		switch (num)
		{
		case 1:
			scanf("%s", str);
			Set(hash, str);									//입력
			break;
		case 2:
			scanf("%s", str);
			node = Get(hash, str);							//검색

			if (node == nullptr)							//오류 방지
			{
				printf("null\n");								
				break;							
			}

															//출력
			printf("문자열 : %s, 키 : %d, 고유키 : %d\n", node->str, node->key, node->hiddenkey);
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