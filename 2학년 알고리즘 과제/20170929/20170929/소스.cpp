#include <stdio.h>

int main()
{
	int Red, Brown;					
	int Sum;
	int x = 0, y = 0;

	printf("입력 : ");
	scanf("%d %d", &Brown, &Red);

	Sum = Red + Brown;					//전체 블록 갯수 구함

	for (int i = 1; i < Sum / 2; i++)	//전체 블록 갯수의 반만큼 약수들을 찾아서 비교할것(반인 이유는 반이후로는 x와 y의 값이 뒤바뀐것뿐이므로)
	{
		if (Sum%i == 0)					//i가 약수일때만
		{
			x = Sum / i;				//넓이 공식 i * x = Sum
			y = i;						//y는 i
			if ((x - 2)*(y - 2) == Red)	//x-2와 y-2를 직사각형의 넓이공식으로 곱해서 나온값이 Red일때
			{
				printf("%d %d\n", x, y);//출력후
				break;					//반복문 종료
			}
		}
	}
}