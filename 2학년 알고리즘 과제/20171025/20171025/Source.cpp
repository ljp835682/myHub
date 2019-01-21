#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Header.h"

void BubbleSort(Score DataSet[], int Length)
{
	int i = 0;
	int j = 0;
	Score temp;

	for (i = 0; i<Length - 1; i++)
	{
		for (j = 0; j<Length - (i + 1); j++)
		{
			if (DataSet[j].score > DataSet[j + 1].score)
			{
				temp = DataSet[j + 1];
				DataSet[j + 1] = DataSet[j];
				DataSet[j] = temp;
			}
		}
	}
}

void InsertionSort(Score DataSet[], int Length)
{
	int i = 0;
	int j = 0;
	Score value;

	for (i = 1; i<Length; i++)
	{
		if (DataSet[i - 1].score <= DataSet[i].score)
			continue;

		value = DataSet[i];

		for (j = 0; j<i; j++)
		{
			if (DataSet[j].score > value.score)
			{
				memmove(&DataSet[j + 1], &DataSet[j], sizeof(DataSet[0]) * (i - j));
				DataSet[j] = value;
				break;
			}
		}
	}
}

int CompareScore(const void * _elem1, const void * _elem2)
{
	Score * elem1 = (Score*)_elem1;
	Score * elem2 = (Score*)_elem2;

	if (elem1->score > elem2->score)
	{
		return 1;
	}
	else if (elem1->score < elem2->score)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int main(void)
{
	clock_t start1, finish1;
	double  duration1;
	start1 = clock();
	int Length = sizeof DataSet1 / sizeof DataSet1[0];

	BubbleSort(DataSet1, Length);

	finish1 = clock();
	duration1 = (double)(finish1 - start1) / CLOCKS_PER_SEC;
	printf("버블정렬 %f 초입니다.\n", duration1);


	clock_t start2, finish2;
	double  duration2;
	start2 = clock();
	Length = sizeof DataSet2 / sizeof DataSet2[0];

	InsertionSort(DataSet2, Length);

	finish2 = clock();
	duration2 = (double)(finish2 - start2) / CLOCKS_PER_SEC;
	printf("삽입정렬 %f 초입니다.\n", duration2);

	clock_t start3, finish3;
	double  duration3;
	start3 = clock();
	Length = sizeof DataSet3 / sizeof DataSet3[0];

	qsort((void*)DataSet3, Length, sizeof (int), CompareScore);

	finish3 = clock();
	duration3 = (double)(finish3 - start3) / CLOCKS_PER_SEC;
	printf("퀵정렬 %f 초입니다.\n", duration3);

	return 0;
}
