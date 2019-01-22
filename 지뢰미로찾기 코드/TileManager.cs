using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TileManager : MonoBehaviour {

    public bool isOriginal = true;          //복사체인지 아닌지 구별
    float x;                                
    float z;                                //자신의 좌표
    bool isMine = false;                    //자신이 지뢰인지 아닌지 구별

    // Use this for initialization
    void Start()
    {
        x = transform.position.x;
        z = transform.position.z;           //좌표 저장

        if ((x >= 1 && x <= 11) && (z >= 1 && z <= 11))
        {
            isOriginal = false;             //지정된 좌표에 있지않다면 복사체가 아님
        }

        if (!isOriginal)                    //복사체만
        {
            MazeManager Maze = GameObject.Find("MazeManager").GetComponent<MazeManager>();

            if (Maze.GetMazeBlock((int)Mathf.Round(x), (int)Mathf.Round(z)) == 7)   //자신이 지뢰인지 확인
            {   
                isMine = true;                                                      //결과 저장 
                //GetComponent<MeshRenderer>().material.color = Color.blue;         //디버그용
            }
        }

        else
        {
            gameObject.SetActive(false);    //오리지널 비활성화
        }
    }
	
	// Update is called once per frame
	void Update ()
    {
        if (!isOriginal)
        {
            PlayerMovement playerMovement = GameObject.Find("Player").GetComponent<PlayerMovement>();

            if (Mathf.Round(playerMovement.GetPositionX()) == x && Mathf.Round(playerMovement.GetPositionZ()) == z)     //플레이어가 자신을 밝으면
            {
                if (isMine)
                {
                    GetComponent<MeshRenderer>().material.color = Color.red;                                            //지뢰일경우 빨강
                }

                else
                {
                    GetComponent<MeshRenderer>().material.color = Color.yellow;                                         //아닐경우 노랑
                }
            }
        }
	}
}
