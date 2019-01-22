using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.AI;
using UnityEngine.SceneManagement;

public class EnemyMovement : MonoBehaviour {

    public Transform startPos;              //시작지점
    public Transform evidence;              //플레이어 캐릭터의 흔적

    //현재 상테에따른 마테리얼값
    public Material mode0;
    public Material mode1;
    //public Material mode2;
    public Material mode3;

    private bool isPlayed = false;          //음악재생을 한번만 하기위한 변수
    public BGMPlayer bgmPlayer;             //음악 재생

    
    private Transform patrolPoints;         //순찰 지점을 모아둔 부모모브젝트
    private Transform cube;                 //자신의 색을 바꾸기위한 변수

    private NavMeshAgent navi;              //네비매쉬
    private Transform player;               //플레이어 오브젝트
    private int mode = 0;                   //현제 상태

    private float stopTime = 3;             //정지할 시간
    private int patrolIndex = 0;            //가야할 순찰 지점의 번호
    

	//초기화
	void Start () {
        transform.position = startPos.position;
        player = GameObject.FindGameObjectWithTag("Player").transform;
        navi = GetComponent<NavMeshAgent>();
        patrolPoints = GameObject.FindGameObjectWithTag("PatrolPoints").transform;
        cube = transform.Find("Cube");
    }
	
	void Update ()
    {
        switch (mode)
        {
            case 0:
            case 1:
            case 3:
                RaycastHit hit;
                if (Physics.Linecast(transform.position, player.position, out hit, 1 << 8))
                {
                    Debug.DrawLine(transform.position, player.position);            //디버그용 선긋기
                    if (hit.collider.tag == "Player")                               //라인캐스터로 닿은 오브젝트가 플레이어일 경우 발견상태로 전환
                    {
                        mode = 1;

                        if (!isPlayed)
                        {
                            isPlayed = true;
                            bgmPlayer.FoundBGM();
                        }

                        navi.speed = 7;
                        cube.GetComponent<MeshRenderer>().material = mode1;
                        evidence.gameObject.SetActive(true);
                        evidence.position = player.position;
                    }
                }
                break;
        }
    }

    void OnTriggerEnter(Collider other)
    {
        //순찰지점과 충돌할경우 다음 순찰 지점으로
        if (other.tag == "PatrolPoint" && mode == 0)
        {
            patrolIndex++;
        }

        //흔적에 닿앗을경우 정지상태로
        else if(other.tag == "Evidence" && mode == 1)
        {
            mode = 2;
            //cube.GetComponent<MeshRenderer>().material = mode2;
        }

        //플레이어에 닿을경우 게임을 다시시작
        else if(other.tag == "Player")
        {
            SceneManager.LoadScene(0);
        }
    }
    void LateUpdate()
    {
        switch (mode)
        {
            case 0:
                //패트롤 상태

                //순찰번호가 범위를 넘을경우 0번으로 초기화
                if (patrolIndex >= patrolPoints.childCount)
                {
                    patrolIndex = 0;
                }

                //다음 순찰지역으로 이동
                navi.SetDestination(patrolPoints.GetChild(patrolIndex).position);
                break;
            case 1:
                //흔적위치로 이동상태
                navi.SetDestination(evidence.position);
                break;
            case 2:
                //정지직전상태
                isPlayed = false;
                evidence.gameObject.SetActive(false);
                navi.SetDestination(transform.position);
                mode = 3;
                cube.GetComponent<MeshRenderer>().material = mode3;
                bgmPlayer.NotFoundBGM();
                break;

            case 3:
                //정지상태
                
                //타이머
                stopTime -= Time.deltaTime;

                //타이머 끝남
                if (stopTime <= 0)
                {
                    mode = 0;
                    navi.speed = 3.5f;
                    cube.GetComponent<MeshRenderer>().material = mode0;
                    stopTime = 3;
                    DistanceMeasurement();
                }

                break;
        }
    }

    //가장 가까운 패트롤 포인트를 찾아내는 함수
    private void DistanceMeasurement()
    {
        int index = 0;
        float dis = Mathf.Infinity;

        for (int i = 0; i < patrolPoints.childCount; i++)
        {
            float tempdis = Vector3.Distance(transform.position, patrolPoints.GetChild(i).position);

            if (tempdis <= dis)
            {
                index = i;
                dis = tempdis;
            }
        }

        if(Vector3.Distance(patrolPoints.GetChild(index).position,transform.position) <= 3)
        {
            index++;
        }

        patrolIndex = index;
    }

    public void Disable()
    {
        gameObject.SetActive(false);
    }
}
