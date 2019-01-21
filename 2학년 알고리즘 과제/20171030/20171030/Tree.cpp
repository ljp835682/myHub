#include "Tree.h"

//노드를 추가하는 함수
void AddNode(Tree * _tree, Node * _visit, int _data)
{
	if (_tree->count == 0)					//만약 첫번쨰 입력시
	{
		_visit->data = _data;				//그냥 대입시킵니다
		_tree->count++;
		return;								//함수 종료
	}

	else									//그외의 경우
	{		
		if (_visit->data > _data)			//만약 방문한 노드의 데이터보다 입력된 데이터가 작다면
		{
			if (_visit->left == nullptr)	//만약 방문한 노드의 왼쪽 자식이 관측되지 않는다면
			{
				Node * newNode = new Node;
				newNode->data = _data;
				newNode->left = nullptr;
				newNode->right = nullptr;
				_visit->left = newNode;		//새로운 노드를 만들고 거기에 대입
				_tree->count++;
			}

			else							//만약 방문한 노드의 왼쪽 자식이 관측되었다면
			{
				AddNode(_tree, _visit->left, _data);
			}								//해당 노드로 이동
		}

		else if (_visit->data < _data)		//만약 방문한 노드의 데이터보다 입력된 데이터가 크다면
		{
			if (_visit->right == nullptr)	//만약 방문한 노드의 오른쪽 자식이 관측되지 않는다면
			{
				Node * newNode = new Node;
				newNode->data = _data;
				newNode->left = nullptr;
				newNode->right = nullptr;		
				_visit->right = newNode;	//새로운 노드를 만들고 거기에 대입
				_tree->count++;
			}

			else							//만약 방문한 노드의 오른쪽 자식이 관측되었다면
			{
				AddNode(_tree, _visit->right, _data);
			}								//해당 노드로 이동
		}
	}
}

//전위순회 출력함수
void PreOrderTreePrint(Tree * _tree, Node * _visit)
{
	if (_visit == nullptr)							//만약 방문한 노드가 존재하지 않는 노드면 함수종료
	{
		return;
	}

	else
	{
		printf(" %d ", _visit->data);				//자신을 먼저 출력
		PreOrderTreePrint(_tree, _visit->left);		//자신의 왼쪽 자식을 출력하기위해 재귀함수 호출
		PreOrderTreePrint(_tree, _visit->right);	//자신의 오른쪽 자식을 출력하기위해 재귀함수 호출
	}
}

//중위순회 출력함수
void InOrderTreePrint(Tree * _tree, Node * _visit)
{
	if (_visit == nullptr)							//만약 방문한 노드가 존재하지 않는 노드면 함수종료
	{
		return;
	}

	else
	{
		InOrderTreePrint(_tree, _visit->left);		//자신의 왼쪽 자식을 출력하기위해 재귀함수 호출
		printf(" %d ", _visit->data);				//자신을 중앙에 출력
		InOrderTreePrint(_tree, _visit->right);		//자신의 오른쪽 자식을 출력하기위해 재귀함수 호출
	}
}

//후위순회 출력함수
void PostOrderTreePrint(Tree * _tree, Node * _visit)
{
	if (_visit == nullptr)							//만약 방문한 노드가 존재하지 않는 노드면 함수종료
	{	
		return;
	}

	else
	{
		PostOrderTreePrint(_tree, _visit->left);	//자신의 왼쪽 자식을 출력하기위해 재귀함수 호출
		PostOrderTreePrint(_tree, _visit->right);	//자신의 오른쪽 자식을 출력하기위해 재귀함수 호출
		printf(" %d ", _visit->data);				//자신을 마지막에 출력
	}
}

//트리삭제함수
void DestroyTree(Tree * _tree, Node * _visit)
{
	if (_visit == nullptr)							//만약 방문한 노드가 존재하지 않는 노드면 함수종료
	{
		return;
	}
													//트리의 삭제를 하려면 맨아래의 노드부터 삭제할 필요가 잇으므로
													//후위순회를 하여 삭제를 하는것이 적합하다
	else
	{
		DestroyTree(_tree, _visit->left);			//자신의 왼쪽 자식을 삭제하기위해 재귀함수 호출		
		DestroyTree(_tree, _visit->right);			//자신의 오른쪽 자식을 삭제하기위해 재귀함수 호출
		delete _visit;								//자신을 마지막에 삭제
		_visit = nullptr;
	}									
}