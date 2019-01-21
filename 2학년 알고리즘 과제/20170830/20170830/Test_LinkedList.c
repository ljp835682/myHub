#include "LinkedList.h"

/*
�����ϸ�
1. �Է� (�ǵڷ�)
2. ����
3. ����
4. ����
5. ���
6. ����

1�����ý�
��ȣ�� ������ �Է¹���

2�����ý�
��ȣ�� �˻��ؼ� ����

4�����ý�
��ȣ�� �˻��ؼ� �� �ڿ� �߰�
*/

int MenuPrint()			//�޴� ��¿� �Լ��Դϴ�
{
	int Choice = 0;

	printf("1. �Է�\n");
	printf("2. ����\n");
	printf("3. ����\n");
	printf("4. ����\n");
	printf("5. ���\n");
	printf("6. ����\n");
	printf("���� : ");		//���������
	scanf("%d", &Choice);	//����ڿ��� �Է��� �ް�
	return Choice;			//�Է¹��� ���ڸ� �ܺη� �����ϴ�
}
int main( void )
{
    int   Count   = 0;		//����� �Ѱ����� �� ������ ����
    Node* List    = NULL;	//����Ʈ�� ���
    Node* Current = NULL;	//���ο� ��带 ���� ����
	Node * FindNode = NULL;
	int flag = 0;			//���� �÷���
	int Number;				//��ȣ�� �Է½�ų ������ ����
	int Score;				//������ �Է½�ų ������ ����

	while (1)
	{
		Number = 0;	
		Score = 0;			//�ʱ�ȭ

		switch (MenuPrint())		//�޴����� �Է¹��� ���ڷ� switch���� �����մϴ�
		{
		case 1:						
			printf("��ȣ : ");
			scanf("%d", &Number);
			printf("���� : ");
			scanf("%d", &Score);		//��ȣ�� ������ �Է¹޽��ϴ�

			Current = SLL_CreateNode(Number, Score);		//���ο� ��带 ������
			SLL_AppendNode(&List, Current);					//�ǳ��� �߰���ŵ�ϴ�
			break;
		case 2:
			printf("������ ��ȣ : ");						
			scanf("%d", &Number);							//������ ��ȣ�� �Է¹޽��ϴ�

			FindNode = SLL_SearchNode(List, Number);		//�Է��� ��ȣ���ִ��� �˻��մϴ�

			if (FindNode != NULL)								//���̾ƴ϶��
			{
				SLL_RemoveNode(&List, FindNode);				//�����մϴ�
			}

			else
			{
				printf("���� ��ȣ\n");							//���̸� �Է��� ��ȣ�� ���»�Ȳ�Դϴ�
			}

			break;
		case 3:
			printf("������ ��ȣ : ");								
			scanf("%d", &Number);									//������ ��ȣ�� �Է¹޽��ϴ�

			FindNode = SLL_SearchNode(List, Number);				//�Է��� ��ȣ�� �ִ��� �˻��մϴ�

			if (FindNode != NULL)									//���̾ƴ϶��
			{
				printf("�����ϱ� �� ���� : %d\n", FindNode->Score);	//�����ϱ��� ������ �����ݴϴ�
				printf("���� �Է� : ");
				scanf("%d", &FindNode->Score);						//���ο� ������ �Է¹޽��ϴ�
			}

			else
			{
				printf("���� ��ȣ\n");								//���̸� �Է��� ��ȣ�� ���»�Ȳ�Դϴ�
			}

			break;
		case 4:
			printf("������ �� ��ȣ : ");
			scanf("%d", &Number);								//�����ϴµ� �����̵� ��ȣ�� �Է¹޽��ϴ�

			FindNode = SLL_SearchNode(List, Number);			//�Է��� ��ȣ�� �ִ��� ã�ƺ��ϴ�

			if (FindNode != NULL)								//���� �ƴ϶��
			{
				printf("��ȣ : ");
				scanf("%d", &Number);
				printf("���� : ");
				scanf("%d", &Score);							//������ �Է¹ް�

				Current = SLL_CreateNode(Number, Score);		//���ο� ��带 ������
				Current->NextNode = FindNode->NextNode;			//�׳���� ������带 ������ �Ǵ� ����� �������� �������ְ�
				FindNode->NextNode = Current;					//������ �Ǵ� ����� ������带 ���θ��� ���� �������ݴϴ�
			}

			else
			{
				printf("���� ��ȣ\n");
			}
			
			break;
		case 5:
			Count = SLL_GetNodeCount(List);						//��� ����� ���ڸ� ����
			for (int i = 0; i<Count; i++ )						//��� ����� ���ڸ�ŭ �ݺ����Ѽ�
			{
			    Current = SLL_GetNodeAt(List, i);
				printf("%d���� ���� : %d\n", Current->Number, Current->Score);		//��� ��½�ŵ�ϴ�
			}
			break;
		case 6:
			flag = 1;					//�����÷����Դϴ�
			break;
		}

		if (flag == 1)					//���� ���ǹ��Դϴ�
		{
			break;						//�ݺ����� ���� ���α׷��� �����ϰԵ˴ϴ�
		}
	}
    return 0;
}


