#include "Tree.h"

//키 생성 함수
int Key(char * _str)								
{
	int newKey = 0;

	for (int i = 0; i < strlen(_str); i++)
	{
		newKey += _str[i];							//유니코드를 전부 더함
	}

	return newKey;
}

//고유키 생성 함수
int HiddenKey(char * _str)							
{
	char str[255] = "";
	char temp[10] = "";

	for (int i = 0; i < strlen(_str); i++)			
	{
		sprintf(temp, "%d", int(_str[i]));			//유니코드를 전부 이어붙혀서 하나의 숫자로 만듬
		strcat(str, temp);							//예 ) 문자열 "abc" -> 정수 979899
	}												//너무 큰 수라 해쉬테이블에는 사용하지않음

	return atoi(str);								
}

//노드 생성 함수
sNode * CreateNode(char * _str)						
{
	sNode * newNode = new sNode;			

	strcpy_s(newNode->str, _str);
	newNode->left = nullptr;
	newNode->right = nullptr;
	newNode->key = Key(_str);
	newNode->hiddenkey = HiddenKey(_str);			//전부 초기화

	return newNode;	
}

//노드를 추가하는 함수
void AddNode(sTree * _tree, sNode * _visit, sNode * _newNode)
{
	if (_tree->count == 0)									//만약 첫번째 입력시
	{
		_tree->root = _newNode;
		_tree->count++;
		return;												//함수 종료
	}

	else													//그외의 경우
	{
		if (_visit->hiddenkey > _newNode->hiddenkey)		//히든키기준 작을경우
		{
			if (_visit->left == nullptr)
			{
				_visit->left = _newNode;					//입력
				_tree->count++;
			}

			else
			{
				AddNode(_tree, _visit->left, _newNode);		//이미 있는 노드일경우 그 노드 아레로
			}
		}

		else if (_visit->hiddenkey < _newNode->hiddenkey)	//히든키기준 클경우
		{
			if (_visit->right == nullptr)
			{
				_visit->right = _newNode;					//입력
				_tree->count++;
			}

			else
			{
				AddNode(_tree, _visit->right, _newNode);	//이미 있는 노드일경우 그 노드 아레로
			}
		}
	}
}

//노드 검색 함수
sNode * SearchNode(sTree * _tree, sNode * _visit, int _hiddenKey)
{
	if (_visit == nullptr)
	{
		return nullptr;
	}

	else
	{
		if (_hiddenKey == _visit->hiddenkey)						//고유키가 같을경우
		{
			return _visit;											//함수종료
		}

		else if (_visit->hiddenkey > _hiddenKey)					//작을경우
		{	
			return SearchNode(_tree, _visit->left, _hiddenKey);		//왼쪽 노드로 이동
		}

		else if (_visit->hiddenkey < _hiddenKey)					//클경우
		{
			return SearchNode(_tree, _visit->right, _hiddenKey);	//오른쪽 노드로 이동
		}
	}
}

//트리삭제함수
void DestroyTree(sTree * _tree, sNode * _visit)
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