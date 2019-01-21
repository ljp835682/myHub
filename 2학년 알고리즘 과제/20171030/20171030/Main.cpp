#include "Tree.h"

int main()
{
	Tree * tree = new Tree;								//트리 생성
	tree->count = 0;									//카운트 초기화
	tree->root = new Node;								//루트 생성
	tree->root->left = nullptr;						
	tree->root->right = nullptr;						//초기화

	int inputNodeCount;
	printf("몇개의 노드를 생성하시겠습니까? : ");
	scanf("%d", &inputNodeCount);						//노드갯수 입력

	int intputData;
	for (int i = 0; i < inputNodeCount; i++)			
	{
		printf("입력 : ");
		scanf("%d", &intputData);
		AddNode(tree, tree->root, intputData);			//노드 추가
	}

	printf("전위 순회법 : ");
	PreOrderTreePrint(tree, tree->root);				//전위 순회법 출력
	printf("\n중위 순회법 : ");
	InOrderTreePrint(tree, tree->root);					//중위 순회법 출력
	printf("\n후위 순회법 : ");
	PostOrderTreePrint(tree, tree->root);				//후위 표기법 출력
	printf("\n");

	DestroyTree(tree, tree->root);						//트리 삭제
}