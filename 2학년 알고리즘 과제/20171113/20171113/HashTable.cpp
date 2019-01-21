#include "HashTable.h"

//키 변환 함수
int KeyTrans(int _key)					
{
	return _key % 199;						//소수 199 미만의 수로 변환
}

//해쉬테이블 생성 함수
sHashTable * CreateHashTable()
{
	sHashTable * hash = new sHashTable;		//해쉬생성
	hash->tree = new sTree *[199];			//199칸 배열생성

	for (int i = 0; i < 199; i++)			
	{
		hash->tree[i] = nullptr;			//전부 초기화
	}

	return hash;
}

//트리 생성 함수
sTree * CreateTree()						
{
	sTree * tree = new sTree;				//트리 생성
	tree->root = nullptr;					
	tree->count = 0;						//초기화

	return tree;
}

//입력 함수
void Set(sHashTable * _hash, char * _str)											
{
	sNode * newNode = CreateNode(_str);										//새로운 노드 생성
	int index = KeyTrans(newNode->key);										//키 변환
	
	if (_hash->tree[index] == nullptr)										//만약 해당 트리의 첫번째 노드라면
	{
		_hash->tree[index] = CreateTree();									//일단 트리를 생성
	}

	AddNode(_hash->tree[index], _hash->tree[index]->root, newNode);			//그리고 트리에 새로운 노드를 담
}

//출력 함수
sNode * Get(sHashTable * _hash, char * _str)
{
	int index = KeyTrans(Key(_str));														//문자열을 기준으로 저장되어있을 공간을 찾아냄
	sNode * node = nullptr;

	if (_hash->tree[index] != nullptr)														//오류방지
	{
		node = SearchNode(_hash->tree[index], _hash->tree[index]->root, HiddenKey(_str));	//있을만한 곳에서 찾기 시작

		if (node != nullptr)																//찾는데 성공했다면
		{
			return node;																	//성공
		}
	}

	return nullptr;																			//실패
}

void DestroyHashTable(sHashTable * _hash)													//해쉬 테이블 삭제
{
	for (int i = 0; i < 199; i++)
	{
		if (_hash->tree[i] != nullptr)														//비지 않은 곳만
		{
			DestroyTree(_hash->tree[i], _hash->tree[i]->root);								//트리의 노드 삭제
			delete _hash->tree[i];															//트리 삭제
		}
	}

	delete _hash->tree;																		//트리배열 삭제
	delete _hash;																			//해쉬테이블 삭제
}