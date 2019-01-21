#include "Tree.h"

struct sHashTable				//해쉬 테이블 구조체
{
	sTree ** tree;				//트리의 포인터 배열(사용하는곳만 동적할당하고 사용하지 않는곳은 4바이트만 할당)
};

sHashTable * CreateHashTable();						//해쉬테이블 생성 함수
sTree * CreateTree();								//트리 생성 함수
sNode * Get(sHashTable * _hash, char * _str);		//겟
void Set(sHashTable * _hash, char * _str);			//셋
int KeyTrans(int _key);								//키 변환
void DestroyHashTable(sHashTable * _hash);			//해쉬 테이블 삭제

