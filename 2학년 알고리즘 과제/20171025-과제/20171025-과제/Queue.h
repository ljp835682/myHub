#include <stdio.h>
#include <string.h>

struct sNode			//��� ����ü
{
	char data;
	int priority;
	sNode * next;
};

struct sQueue			//����ť ����ü
{
	sNode * front;
	sNode * rear;
	int mSize;
	int mCount;
};

void CreateQueue(sQueue * _queue, int _size);				//ť ����
bool EnQueue(sQueue * _queue, char _data, int _priority);	//ť�� ���ο� ������ ����ֱ�
sNode * DeQueue(sQueue * _queue);							//ť�� �Ǿ� ������ ������
bool IsMinCheck(sQueue * _queue, int _priority);			//ť�� ���� ������ ���ε����Ϳ� _priority ���� ũ�⸦ ���ϴ� �Լ�
void DestroyQueue(sQueue * _queue);							//ť ����