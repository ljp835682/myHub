using System.Collections;
using System.Collections.Generic;
using System.Threading;
using UnityEngine;

public class PlayerMovement : MonoBehaviour
{
    public float length = 1f;               //큐브의 모서리의 길이
    private float rotationPeriod = 0.3f;     //회전에 걸릴 시간
    private bool isRotate = false;          //큐브가 회전중인지 아닌지를 보는 플래그
    private float mX = 0;                   //x축 회전시 사용될 변수
    private float mZ = 0;                   //z축 회전시 사용된 변수

    Vector3 startPos;                       //회전전 큐브의 위치
    float rotationTime = 0;                 //회전중의 시간경과
    float radius;                           //큐브가 회전하면서 생기는 원형 궤도의 반경

    Quaternion BeforeRo;                    //회전전 Quaternion
    Quaternion AfterRo;                     //회전후 Quaternion

    // Use this for initialization

    public bool GetisRotate()
    {
        return isRotate;
    }

    public float GetPositionX()
    {
        return transform.position.x;
    }

    public float GetPositionZ()
    {
        return transform.position.z;
    }

    public bool GotoOldPosition()
    {
        transform.position = startPos;
        return true;                    //나중을 위한 bool형 리턴
    }

    void Start ()
    {
        MazeCreater mazecreator = GameObject.Find("MazeCreater").GetComponent<MazeCreater>();
        transform.position = new Vector3(mazecreator.GetStartX(), 0.5f, mazecreator.GetStartY());

        //중심 회전 궤도 반경 계산
        radius = length * Mathf.Sqrt(length * 2) / (length * 2);
	}

    // Update is called once per frame
    void Update()
    {

        float x = 0;
        float y = 0;

        if (Input.GetKeyDown(KeyCode.UpArrow))
        {
            y = 1;

            if (Mathf.Round(transform.position.x) + 1 > 11)      //상
            {
                y = 0;
            }
        }

        else if (Input.GetKeyDown(KeyCode.DownArrow))
        {
            y = -1;

            if (Mathf.Round(transform.position.x) - 1 < 1)     //하
            {
                y = 0;
            }
        }

        else if (Input.GetKeyDown(KeyCode.LeftArrow))
        {
            x = -1;

            if (Mathf.Round(transform.position.z) + 1 > 11)     //좌
            {
                x = 0;
            }
        }

        else if (Input.GetKeyDown(KeyCode.RightArrow))
        {
            x = 1;

            if (Mathf.Round(transform.position.z) - 1 < 1)      //우
            {
                x = 0;
            }
        }

        x *= -1;
        y *= -1;

        if ((x != 0 || y != 0) && !isRotate)
        {
            mX = y;
            mZ = x;                                             //회전방향 지정

            startPos = transform.position;                      //회전전의 위치좌표 저장
            BeforeRo = transform.rotation;                      //회전전의 Quaternion
            transform.Rotate(mZ * 90, 0, mX * 90, Space.World); //지정된 방향으로 회전(사용되지 않는쪽의 반향을 가르키는 변수는 0일것)
            AfterRo = transform.rotation;                       //회전후의 Quaternion
            transform.rotation = BeforeRo;                      //다시 이전상태로 돌아감
            rotationTime = 0;                                   //회전중 경과시간 0으로 초기화
            isRotate = true;                                    //회전중이라는 플래그를 세움
        }
    }

    void FixedUpdate()
    {
        if (isRotate)
        {
            rotationTime += Time.fixedDeltaTime;                                //경과시간을 증가시킴
            float ratio = Mathf.Lerp(0, 1, rotationTime / rotationPeriod);      //회전시간에 대한 경과시간의 비율

            //이동
            float thetaRad = Mathf.Lerp(0, Mathf.PI / 2f, ratio);                   
            float distanceX = -mX * radius * (Mathf.Cos(45f * Mathf.Deg2Rad) - Mathf.Cos(45f * Mathf.Deg2Rad + thetaRad));  //x축 이동거리(-를붙히지 않으면 키와 반대로 감)
            float distanceY = radius * (Mathf.Sin(45f * Mathf.Deg2Rad + thetaRad) - Mathf.Sin(45f * Mathf.Deg2Rad));        //y축 이동거리
            float distanceZ = mZ * radius * (Mathf.Cos(45f * Mathf.Deg2Rad) - Mathf.Cos(45f * Mathf.Deg2Rad + thetaRad));   //z축 이동거리
            transform.position = new Vector3(startPos.x + distanceX, startPos.y + distanceY, startPos.z + distanceZ);       //현재 위치 저장

            //회전
            transform.rotation = Quaternion.Lerp(BeforeRo, AfterRo, ratio);     //경과시간의 비율만큼 회전

            if (ratio == 1)                //회전비율이 100% 즉 회전이 끝낫을때
            {
                isRotate = false;
                mX = 0;
                mZ = 0;
                rotationTime = 0;          //멤버 변수 초기화
            }
        }
    }
}
