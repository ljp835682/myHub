#include "Queue.h"

void CreateQueue(sQueue * _queue, int _size)
{
	_queue->front = nullptr;
	_queue->rear = nullptr;
	_queue->mSize = _size;
	_queue->mCount = 0;

	for (int i = 0; i < _size + 1; i++)					//�ι��� ���ڰ� + 1 ��ŭ �ݺ�
	{
		sNode * newNode = new sNode;					//���ο� ��� ����
		newNode->data = NULL;
		newNode->priority = NULL;
		newNode->next = nullptr;

		if (i == 0)										//ù��° ��Ȳ�� ���
		{
			_queue->front = newNode;					//�ش� ��带 front�� ����
		}

		else if (i == 1)								//�ι�° ��Ȳ�� ���
		{
			_queue->front->next = newNode;				//�ش� ��带 front�� ������ ��ġ
			_queue->rear = newNode;						//�ش� ��带 rear�� ����
		}

		else											//�׿��� ��Ȳ�� ���
		{
			_queue->rear->next = newNode;				//�ش� ��带 rear ������ ��ġ
			_queue->rear = newNode;						//�ش� ��带 rear�� ����
		}
	}

	_queue->rear->next = _queue->front;					//�ݺ����� ���� ������ Ƚ����ŭ ������ �����ǰ��� �� ������ ����� ������ front�� ����
	_queue->rear = _queue->front->next;					//���� ���ο� �����Ͱ� ���ð��� ���̳���� front�� ���� ���� ����
}

bool EnQueue(sQueue * _queue, char _data, int _priority)
{
	if (_queue->rear == _queue->front)					//���� rear�� front�� ���ٸ�
	{
		return false;									//���� ����ť�� ��ȭ�����ΰ��̹Ƿ� ������ �����մϴ�
	}

	_queue->rear->data = _data;							
	_queue->rear->priority = _priority;					//rear�� �����Ϳ� ����
	_queue->rear = _queue->rear->next;					//���� �Է��� ���� ��忡 �Է��Ҽ��ֵ��� rear ���� ���� �̵���

	_queue->mCount++;									//���� �Էµ� ���� ����

	return true;
}

sNode * DeQueue(sQueue * _queue)
{
	sNode * tempNode = new sNode;						//���ο� ��� ����
	_queue->front = _queue->front->next;				//front�� ���� ���� ���� �̵�

	memcpy(tempNode, _queue->front, sizeof(sNode));		//front�� �����͸� ���ο� ��忡 ���� ����
	
	_queue->front->data = NULL;							
	_queue->front->priority = NULL;						//front�� ������ ����

	_queue->mCount--;									//���� �Էµ� ���� ����

	return tempNode;									//front�� �������� ���������� ��带 �ܺη� ����
}

bool IsMinCheck(sQueue * _queue, int _priority)
{
	sNode * searchNode = _queue->front->next;					//���� ���� ��忡������ ���ϱ����� front�� ���� ���� �˻� ��� ����

	while (1)
	{
		if (searchNode == _queue->front)						//���� �˻���尡 front��� �ѹ����� �ٵ����� �Ʒ����� ���� �����Ѱ��̹Ƿ�
		{
			return true;										//true ����
		}

		else if (searchNode->priority > _priority || searchNode->priority == NULL)		//���� �� ����̰ų� �˻� ��尡 ����Ű�� ����� �켱���� �Էµ� �켱������ ũ�ٸ�
		{
			searchNode = searchNode->next;												//���� ��带 ���� �غ�
		}

		else
		{
			return false;																//�۴ٸ� false ����
		}
	}
}

void DestroyQueue(sQueue * _queue)	
{
	for (int i = 0; i < _queue->mSize; i++)						//ť�� �����ŭ �ݺ�
	{
		sNode * destroyNode = _queue->front;					//������ ��忡 front ����
		_queue->front = _queue->front->next;					//front�� ���� ���� �̵�

		delete destroyNode;										//������ ��� ��ȯ
	}
}