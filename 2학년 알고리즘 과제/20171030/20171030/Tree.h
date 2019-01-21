#include <stdio.h>
#include <stdlib.h>

struct Node						//노드 구조체
{	
	int data;
	Node * left;				//자신의 왼쪽 자식 노드
	Node * right;				//자신의 오른쪽 자식 노드
};

struct Tree						//트리 구조체
{
	Node * root;				//루트 노드
	int count;					//노드의 갯수
};

void AddNode(Tree * _tree, Node * _visit, int _data);		//노드를 추가하는 함수
void PreOrderTreePrint(Tree * _tree, Node * _visit);		//전위순회 출력함수
void InOrderTreePrint(Tree * _tree, Node * _visit);			//중위순회 출력함수
void PostOrderTreePrint(Tree * _tree, Node * _visit);		//후위순회 출력함수
void DestroyTree(Tree * _tree, Node * _visit);				//트리삭제함수