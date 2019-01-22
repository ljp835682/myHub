using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.SceneManagement;

public class MazeManager : MonoBehaviour
{
    public int life = 3;                    //유니티에서 입력할 라이프수치
    static private int lifeCount;           //라이프를 저장할 정적변수
    static private bool lifeflag = true;    //시작했을때만 라이프수치 대입
    static private int score = 0;           //점수를 저장할 정적변수
    public Text lifeText;                   //남은 목숨을 출력할 UI  
    public Text scoreText;                  //점수를 출력할 UI
    public GameObject mineSearcher;         //지뢰 탐색기 게이지 게임오브젝트들의 부모
    private GameObject[] mineGauge;         //지뢰 탐색기 게이지 게임오브젝트들을 저장할 배열
    private int[,] maze;                    //미로
    private int playerX;                    //배열상 플레이어의 위치
    private int playerY;                          
    private PlayerMovement playerMovement;  //플레이어 스크립트
    public GameObject Explosion;

    public int GetMazeBlock(int _x,int _y)
    {
        return maze[_x, _y];
    }

    // Use this for initialization
    void Start()
    {
        if (lifeflag)
        {
            lifeCount = life;
            lifeflag = false;
        }

        playerMovement = GameObject.Find("Player").GetComponent<PlayerMovement>();              

        MazeCreater mazecreator = GameObject.Find("MazeCreater").GetComponent<MazeCreater>();
        maze = mazecreator.GetMaze();
        playerX = mazecreator.GetStartX();
        playerY = mazecreator.GetStartY();  //플레이어 좌표 설정

        mineGauge = new GameObject[7];      //7칸 배열 생성
        for (int i = 0; i < 7; i++)
        {
            mineGauge[i] = mineSearcher.transform.Find("MineSearchGauge" + i.ToString()).gameObject;        //자식에서 일치하는 이름을 가진 게임오브젝트 대입
            mineGauge[i].SetActive(false);                                                                  //안보이도록 설정
        }
    }

    // Update is called once per frame
    void Update()
    {
        scoreText.text = "Score : " + score.ToString();
        lifeText.text = "Life : " + lifeCount.ToString();

        if (playerMovement.GetisRotate())                           //플레이어 이동중
        {
            for (int i = 0; i < 7; i++)
            {
                mineGauge[i].SetActive(false);                      //마인게이지 전부 비활성화
            }
        }

        else
        {

            playerX = (int)(Mathf.Round(playerMovement.GetPositionX()));
            playerY = (int)(Mathf.Round(playerMovement.GetPositionZ()));        //플레이어 이동완료시 배열상 플레이어위치 변경

            int mineCount = 0;

            if (maze[playerX, playerY + 1] == 7)        //7은 마인
            {
                mineCount++;
            }

            if (maze[playerX + 1, playerY + 1] == 7)
            {
                mineCount++;
            }

            if (maze[playerX + 1, playerY] == 7)
            {
                mineCount++;
            }

            if (maze[playerX + 1, playerY - 1] == 7)
            {
                mineCount++;
            }

            if (maze[playerX, playerY - 1] == 7)
            {
                mineCount++;
            }

            if (maze[playerX - 1, playerY + 1] == 7)
            {
                mineCount++;
            }

            if (maze[playerX - 1, playerY] == 7)
            {
                mineCount++;
            }

            if (maze[playerX - 1, playerY - 1] == 7)
            {
                mineCount++;
            }                               //활성화할 마인 게이지 갯수 확인을 위해 전방위 검사

            for (int i = 0; i < mineCount; i++)
            {
                mineGauge[i].SetActive(true);           //주변 8방위중 마인갯수만큼 활성화
            }

            if (maze[playerX, playerY] == 7)            //마인을 밝은경우
            {
                Explosion.SetActive(false);
                Explosion.SetActive(true);
                lifeCount--;                            //라이프 감소

                if (playerMovement.GotoOldPosition())   //이전포지션으로 이동
                {
                    playerX = (int)(Mathf.Round(playerMovement.GetPositionX()));
                    playerY = (int)(Mathf.Round(playerMovement.GetPositionZ()));        //플레이어 이동완료시 배열상 플레이어위치 변경
                }

                if (lifeCount == -1)                    //라이프 전부 소진시
                {
                    lifeCount = life;                   //라이프 다시 초기화
                    score = 0;
                    SceneManager.LoadScene("MainGame"); //게임 다시시작
                }
            }

            if (maze[playerX, playerY] == 10)           //플레이어가 골지점에 도달할경우 (10은 골지점)
            {
                score += 100;
                SceneManager.LoadScene("MainGame");     //게임 다시시작
            }
        }
    }
}