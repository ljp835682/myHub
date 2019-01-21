#include "GraphTraversal.h"

//깊이 우선 탐색
void DFS(Vertex* V)
{
	Edge* E = NULL;						

	printf("%d ", V->Data);						//현재 정점의 데이터를 출력

	V->Visited = Visited;						//방문 상태로 변경

	E = V->AdjacencyList;						//간선은 현재 정점의 간선으로 지정

	//간선이 있는동안 반복
	while (E != NULL)
	{
		//만약 현재 간선의 타겟정점이 있고 해당 타겟정점의 상태가 방문되지 않은 상태일경우
		if (E->Target != NULL && E->Target->Visited == NotVisited)
			DFS(E->Target);						//재귀호출

		E = E->Next;							//간선은 다음 간선으로 이행
	}
}

//너비 우선 탐색
void BFS(Vertex* V, LinkedQueue* Queue)
{
	Edge* E = NULL;

	printf("%d ", V->Data);						//현재 정점의 데이터 출력
	V->Visited = Visited;						//현재 정점의 상태를 방문된 상태로 변경

	/*  큐에 노드 삽입. */
	LQ_Enqueue(&Queue, LQ_CreateNode(V));		

	//큐가 비지 않을 동안 반복
	while (!LQ_IsEmpty(Queue))
	{
		Node* Popped = LQ_Dequeue(&Queue);		//맨앞의 노드를 꺼냄
		V = Popped->Data;						//정점은 팝된 노드의 데이터
		E = V->AdjacencyList;					//간선은 팝된 노드의 정점의 간선

		//간선이 비지 않을때까지 반복
		while (E != NULL)
		{
			V = E->Target;						//정점은 간선의 타겟으로 지정

			//만약 정점이 비지 않았고 정점의 상태가 방문되지 않은 상태일경우
			if (V != NULL && V->Visited == NotVisited)
			{
				printf("%d ", V->Data);			//출력
				V->Visited = Visited;			//상태 변경
				LQ_Enqueue(&Queue, LQ_CreateNode(V));		//새로운 데이터 큐에 삽입
			}

			E = E->Next;						//간선은 다음 간선으로 이행
		}
	}
}