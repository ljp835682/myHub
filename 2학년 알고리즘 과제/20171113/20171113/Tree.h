#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRINGLEN 40

struct sNode
{
	int key;					//자신의 주소를 정할 자신의 키
	int hiddenkey;				//충돌시 사용할 자신의 고유키 (문자혹은 숫자 혹은 특수기호를 받는한 절대 다른 문자열과 같은 값이 나올수없다)
	char str[STRINGLEN];		//저장된 문자열
	sNode * left;				//자신의 왼쪽 자식 노드
	sNode * right;				//자신의 오른쪽 자식 노드
};

int Key(char * _str);				//키 생성 함수
int HiddenKey(char * _str);			//고유키 생성 함수
sNode * CreateNode(char * _str);	//노드 생성 함수

struct sTree						//트리 구조체
{
	sNode * root;					//루트 노드
	int count;						//노드의 갯수
};

void AddNode(sTree * _tree, sNode * _visit, sNode * _newNode);		//노드를 추가하는 함수
sNode * SearchNode(sTree * _tree, sNode * _visit, int _hiddenKey);	//노드 탐색
void DestroyTree(sTree * _tree, sNode * _visit);					//트리삭제함수