#include <stdio.h>

int main()
{
	int Red, Brown;					
	int Sum;
	int x = 0, y = 0;

	printf("�Է� : ");
	scanf("%d %d", &Brown, &Red);

	Sum = Red + Brown;					//��ü ��� ���� ����

	for (int i = 1; i < Sum / 2; i++)	//��ü ��� ������ �ݸ�ŭ ������� ã�Ƽ� ���Ұ�(���� ������ �����ķδ� x�� y�� ���� �ڹٲ�ͻ��̹Ƿ�)
	{
		if (Sum%i == 0)					//i�� ����϶���
		{
			x = Sum / i;				//���� ���� i * x = Sum
			y = i;						//y�� i
			if ((x - 2)*(y - 2) == Red)	//x-2�� y-2�� ���簢���� ���̰������� ���ؼ� ���°��� Red�϶�
			{
				printf("%d %d\n", x, y);//�����
				break;					//�ݺ��� ����
			}
		}
	}
}