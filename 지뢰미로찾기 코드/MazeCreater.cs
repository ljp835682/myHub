using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MazeCreater : MonoBehaviour        //랜덤 미로 생성 알고리즘
{

    private int[,] maze;        //미로를 저장할 배열
    private int startX;         //시작지점
    private int startY;
    private int goalX;          //끝지점
    private int goalY;

    public int[,] GetMaze()
    {
        return maze;
    }

    public int GetStartX()
    {
        return startX;
    }

    public int GetStartY()
    {
        return startY;
    }

    public int GetGoalX()
    {
        return goalX;
    }

    public int GetGaolY()
    {
        return goalY;
    }

    // Use this for initialization
    void Start()
    {
        maze = new int[13, 13] {    { 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
                                    { 6, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 6},
                                    { 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6},
                                    { 6, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 6},
                                    { 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6},
                                    { 6, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 6},
                                    { 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6},
                                    { 6, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 6},
                                    { 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6},
                                    { 6, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 6},
                                    { 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6},
                                    { 6, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 6},
                                    { 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6}};      //5 : 0으로 치환할기준점 , 6 : 테두리 , 7 : 부셔지는 벽 

        int baseX;
        int baseY;      //최초의 빈칸을 뚫을 좌표

        while (true)
        {
            baseX = Random.Range(1, 12);
            baseY = Random.Range(1, 12);

            if (maze[baseX, baseY] == 5)
            {
                maze[baseY, baseX] = 0;        
                break;
            }
        }

        int moveX;
        int moveY;

        while (true)
        {
            while (true)
            {
                moveX = Random.Range(1, 12);
                moveY = Random.Range(1, 12);

                if (baseX == moveX && baseY == moveY)
                {
                    continue;
                }

                if (maze[moveX, moveY] == 5)
                {
                    break;
                }
            }

            baseX = 0;
            baseY = 0;

            while (true)                            //랜덤으로 방향성 반복문
            {
                bool Outflag1 = false;
                int direction = Random.Range(1, 5); // 1 : 전 , 2 : 후 , 3 : 좌 , 4 : 우
                switch (direction)
                {
                    case 1:
                        if (maze[moveX, moveY - 1] != 6)
                        {
                            moveY -= 1;
                            maze[moveX, moveY] = 1;
                            moveY -= 1;

                            if (maze[moveX, moveY] == 0)
                            {
                                Outflag1 = true;
                            }

                            maze[moveX, moveY] = 1;
                        }
                        break;
                    case 2:
                        if (maze[moveX, moveY + 1] != 6)
                        {
                            moveY += 1;
                            maze[moveX, moveY] = 2;
                            moveY += 1;

                            if (maze[moveX, moveY] == 0)
                            {
                                Outflag1 = true;
                            }

                            maze[moveX, moveY] = 2;
                        }
                        break;
                    case 3:
                        if (maze[moveX - 1, moveY] != 6)
                        {
                            moveX -= 1;
                            maze[moveX, moveY] = 3;
                            moveX -= 1;

                            if (maze[moveX, moveY] == 0)
                            {
                                Outflag1 = true;
                            }

                            maze[moveX, moveY] = 3;
                        }
                        break;
                    case 4:
                        if (maze[moveX + 1, moveY] != 6)
                        {
                            moveX += 1;
                            maze[moveX, moveY] = 4;
                            moveX += 1;

                            if (maze[moveX, moveY] == 0)
                            {
                                Outflag1 = true;
                            }

                            maze[moveX, moveY] = 4;
                        }                                   
                        break;
                }

                if (Outflag1)
                {
                    break;
                }
            }

            while (true)                    //남긴 방향성의 반대로 최대한 이동
            {
                bool outflag2 = false;
                switch (maze[moveX, moveY])  // 1 : 후 , 2 : 전 , 3 : 우 , 4 : 좌
                {
                    case 1:
                        moveY += 1;
                        maze[moveX, moveY - 1] = 0;
                        break;
                    case 2:
                        moveY -= 1;
                        maze[moveX, moveY + 1] = 0;
                        break;
                    case 3:
                        moveX += 1;
                        maze[moveX - 1, moveY] = 0;
                        break;
                    case 4:
                        moveX -= 1;
                        maze[moveX + 1, moveY] = 0;
                        break;
                    default:
                        maze[moveX, moveY] = 0;
                        outflag2 = true;
                        break;
                }

                if (outflag2)
                {
                    break;
                }
            }

            for (int i = 1; i < 12; i++)                //쓰래기값 처리
            {
                for (int j = 1; j < 12; j++)
                {
                    if (maze[i, j] >= 1 && maze[i, j] <= 4)
                    {
                        if (i % 2 == 1 && j % 2 == 1)
                        {
                            maze[i, j] = 5;
                        }

                        else
                        {
                            maze[i, j] = 7;
                        }
                    }
                }
            }

            bool exitflag3 = true;

            for (int i = 1; i < 12; i += 2)             //5가 더있으면 더돌림, 없으면 미로작성 완료
            {
                for (int j = 1; j < 12; j += 2)
                {
                    if (maze[i, j] == 5)
                    {
                        exitflag3 = false;
                        break;
                    }

                }

                if (!exitflag3)
                {
                    break;
                }
            }

            if (exitflag3)
            {
                break;
            }
        }

        while (true)                                //적당한 곳에 골 배치
        {
            goalX = Random.Range(1, 12);
            goalY = Random.Range(1, 12);
            int count = 0;

            if (maze[goalX, goalY] != 0)
            {
                continue;
            }

            if (maze[goalX + 1, goalY] == 0)
            {
                count++;
            }

            if (maze[goalX - 1, goalY] == 0)
            {
                count++;
            }

            if (maze[goalX, goalY + 1] == 0)
            {
                count++;
            }

            if (maze[goalX, goalY - 1] == 0)
            {
                count++;
            }

            if (count == 1)
            {
                maze[goalX, goalY] = 10;
                break;
            }
        }

        while (true)                                //적당한곳에 시작점 배치
        {
            startX = Random.Range(1, 12);
            startY = Random.Range(1, 12);
            int count = 0;

            if (maze[startX, startY] != 0)
            {
                continue;
            }

            if (maze[startX + 1, startY] == 0)
            {
                count++;
            }

            if (maze[startX - 1, startY] == 0)
            {
                count++;
            }

            if (maze[startX, startY + 1] == 0)
            {
                count++;
            }

            if (maze[startX, startY - 1] == 0)
            {
                count++;
            }

            if (count == 1)
            {
                break;
            }
        }
    }
	
	// Update is called once per frame
	void Update () {
		
	}
}
